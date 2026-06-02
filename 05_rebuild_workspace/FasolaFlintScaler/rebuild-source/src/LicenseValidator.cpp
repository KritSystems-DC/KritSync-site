#include "FasolaFlintScaler/LicenseValidator.h"

#include <windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <ctime>
#include <sstream>

namespace FasolaFlintScaler {
namespace {

constexpr const char* kPrefix = "KFASLO-v1";
constexpr const char* kProduct = "Krit-Faslo";

// Replace this blob with the production BCRYPT_RSAPUBLIC_BLOB bytes used for
// issuing customer licenses. The server plugin must never contain a private key.
constexpr std::array<unsigned char, 0> kPublicKeyBlob{};

std::vector<std::string> Split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, delimiter)) {
        parts.push_back(item);
    }
    return parts;
}

std::vector<unsigned char> HexDecode(const std::string& hex) {
    if (hex.size() % 2 != 0) {
        return {};
    }

    std::vector<unsigned char> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        const auto part = hex.substr(i, 2);
        char* end = nullptr;
        const auto value = std::strtoul(part.c_str(), &end, 16);
        if (end == nullptr || *end != '\0' || value > 0xFF) {
            return {};
        }
        bytes.push_back(static_cast<unsigned char>(value));
    }
    return bytes;
}

bool ParseIsoDate(const std::string& value, std::tm& out) {
    if (value.size() != 10 || value[4] != '-' || value[7] != '-') {
        return false;
    }

    out = {};
    out.tm_year = std::stoi(value.substr(0, 4)) - 1900;
    out.tm_mon = std::stoi(value.substr(5, 2)) - 1;
    out.tm_mday = std::stoi(value.substr(8, 2));
    out.tm_hour = 23;
    out.tm_min = 59;
    out.tm_sec = 59;
    return true;
}

bool VerifySignature(const std::string& message, const std::vector<unsigned char>& signature) {
    if (kPublicKeyBlob.empty() || signature.empty()) {
        return false;
    }

    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    BCRYPT_KEY_HANDLE key = nullptr;
    DWORD hash_length = 0;
    DWORD result_size = 0;
    std::vector<unsigned char> hash_object;
    std::vector<unsigned char> hash_value;

    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) {
        return false;
    }

    auto cleanup = [&]() {
        if (hash) BCryptDestroyHash(hash);
        if (key) BCryptDestroyKey(key);
        if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    };

    if (BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH, reinterpret_cast<PUCHAR>(&hash_length),
                          sizeof(hash_length), &result_size, 0) != 0) {
        cleanup();
        return false;
    }

    DWORD object_length = 0;
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&object_length),
                          sizeof(object_length), &result_size, 0) != 0) {
        cleanup();
        return false;
    }

    hash_object.resize(object_length);
    hash_value.resize(hash_length);

    if (BCryptCreateHash(algorithm, &hash, hash_object.data(), object_length, nullptr, 0, 0) != 0
        || BCryptHashData(hash, reinterpret_cast<PUCHAR>(const_cast<char*>(message.data())),
                          static_cast<ULONG>(message.size()), 0) != 0
        || BCryptFinishHash(hash, hash_value.data(), hash_length, 0) != 0) {
        cleanup();
        return false;
    }

    BCRYPT_ALG_HANDLE rsa = nullptr;
    if (BCryptOpenAlgorithmProvider(&rsa, BCRYPT_RSA_ALGORITHM, nullptr, 0) != 0) {
        cleanup();
        return false;
    }

    if (BCryptImportKeyPair(rsa, nullptr, BCRYPT_RSAPUBLIC_BLOB, &key,
                            const_cast<PUCHAR>(kPublicKeyBlob.data()),
                            static_cast<ULONG>(kPublicKeyBlob.size()), 0) != 0) {
        BCryptCloseAlgorithmProvider(rsa, 0);
        cleanup();
        return false;
    }

    BCRYPT_PKCS1_PADDING_INFO padding{};
    padding.pszAlgId = BCRYPT_SHA256_ALGORITHM;
    const bool valid = BCryptVerifySignature(key, &padding, hash_value.data(), hash_length,
                                             const_cast<PUCHAR>(signature.data()),
                                             static_cast<ULONG>(signature.size()),
                                             BCRYPT_PAD_PKCS1) == 0;

    BCryptCloseAlgorithmProvider(rsa, 0);
    cleanup();
    return valid;
}

}  // namespace

LicenseResult LicenseValidator::Validate(const std::string& token) const {
    if (token.empty()) {
        return {false, "LicenseKey is empty"};
    }

    const auto parts = Split(token, '.');
    if (parts.size() != 5 || parts[0] != kPrefix) {
        return {false, "license token format is invalid"};
    }

    const auto& product = parts[1];
    const auto& version = parts[2];
    const auto& expires = parts[3];
    const auto& signature_hex = parts[4];

    if (product != kProduct) {
        return {false, "license product does not match Krit-Faslo"};
    }

    if (version != "1") {
        return {false, "license version is unsupported"};
    }

    std::tm expiry_tm{};
    try {
        if (!ParseIsoDate(expires, expiry_tm)) {
            return {false, "license expiry date is invalid"};
        }
    } catch (...) {
        return {false, "license expiry date is invalid"};
    }

    const auto expiry_time = std::mktime(&expiry_tm);
    if (expiry_time == -1) {
        return {false, "license expiry date is invalid"};
    }

    const auto now = std::time(nullptr);
    if (now > expiry_time) {
        return {false, std::string("license expired on ") + expires};
    }

    const auto signed_message = parts[0] + "." + product + "." + version + "." + expires;
    const auto signature = HexDecode(signature_hex);
    if (!VerifySignature(signed_message, signature)) {
        return {false, "license signature is invalid"};
    }

    return {true, "valid"};
}

bool LicenseValidator::ConstantTimeEqual(const std::vector<unsigned char>& left,
                                         const std::vector<unsigned char>& right) {
    if (left.size() != right.size()) {
        return false;
    }

    unsigned char diff = 0;
    for (size_t i = 0; i < left.size(); ++i) {
        diff |= left[i] ^ right[i];
    }
    return diff == 0;
}

}  // namespace FasolaFlintScaler
