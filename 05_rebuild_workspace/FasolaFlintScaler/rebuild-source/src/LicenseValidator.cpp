#include "FasolaFlintScaler/LicenseValidator.h"

#include <nlohmann/json.hpp>

#include <windows.h>
#include <bcrypt.h>
#include <wincrypt.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sstream>

namespace FasolaFlintScaler {
namespace {

constexpr const char* kPrefix = "KFASLO-v1";
constexpr const char* kProduct = "Krit-Faslo";

constexpr std::array<unsigned char, 411> kPublicKeyBlob{
    0x52, 0x53, 0x41, 0x31, 0x00, 0x0C, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0xCA, 0x8A, 0xD9, 0x84, 0xAF, 0x81, 0x89, 0x15, 0x5E,
    0x0F, 0x08, 0x5A, 0x35, 0x0C, 0xB6, 0x11, 0x3E, 0x93, 0xA0, 0x43, 0x57,
    0x3C, 0x11, 0xFA, 0x27, 0xC6, 0x9E, 0x41, 0x65, 0x9C, 0x45, 0x30, 0x18,
    0xE7, 0x4A, 0x65, 0xF3, 0x63, 0x7F, 0x67, 0x0C, 0x3B, 0x40, 0xB5, 0xF7,
    0x6E, 0xDA, 0xF8, 0xFB, 0x1C, 0xA5, 0x18, 0x35, 0xAA, 0x0A, 0x37, 0xAD,
    0xC3, 0x48, 0x71, 0x8F, 0xB6, 0x7D, 0x3D, 0xA0, 0x8B, 0x7E, 0xC9, 0x99,
    0xAF, 0x18, 0xDE, 0x77, 0xBB, 0xE0, 0xD3, 0xCB, 0x10, 0x8D, 0xEC, 0xB7,
    0xD2, 0x0F, 0xFC, 0x7E, 0x6C, 0x1F, 0xA0, 0x7F, 0xBE, 0xDD, 0xB8, 0xEC,
    0x87, 0x61, 0x0C, 0x0F, 0x84, 0x0C, 0x38, 0x42, 0xFC, 0xF0, 0xDC, 0x16,
    0xE6, 0xE6, 0x7B, 0x1A, 0xF3, 0xF7, 0x21, 0x0E, 0x09, 0xF0, 0xF0, 0x28,
    0x71, 0x03, 0x24, 0xD7, 0xD8, 0x86, 0xDE, 0x31, 0x44, 0x5A, 0x0C, 0x9F,
    0xA0, 0x4D, 0x93, 0xCA, 0x24, 0x92, 0x1B, 0xC6, 0x71, 0xED, 0xE4, 0x0E,
    0xE6, 0x4A, 0xF1, 0xD2, 0x2C, 0x30, 0x73, 0xB5, 0x06, 0xB2, 0xD7, 0x18,
    0xF9, 0x62, 0x9B, 0x3D, 0xB3, 0xAF, 0x38, 0x2E, 0xD1, 0x8E, 0x73, 0xE8,
    0xCE, 0x63, 0x20, 0x4A, 0x48, 0x12, 0xD7, 0xE0, 0x9C, 0xFE, 0xF8, 0x83,
    0xAB, 0x0D, 0xBB, 0x19, 0xB0, 0x51, 0x28, 0x94, 0x99, 0x0D, 0x5D, 0xA4,
    0xED, 0xF7, 0x38, 0xB2, 0xD5, 0x0A, 0x9A, 0xDB, 0xA0, 0x48, 0x59, 0xFF,
    0x3C, 0x4C, 0x7B, 0x3E, 0x55, 0x69, 0x73, 0xE4, 0xA2, 0xE8, 0x07, 0x96,
    0x3A, 0x97, 0x68, 0xDE, 0xE1, 0x67, 0x9D, 0x62, 0x3B, 0x83, 0x3F, 0x93,
    0x44, 0x33, 0xAA, 0xF1, 0x4A, 0x24, 0xAF, 0x09, 0x49, 0x0C, 0xB6, 0x10,
    0xA8, 0x8F, 0x2E, 0x69, 0xF9, 0x8E, 0x3E, 0x7A, 0xDB, 0xBE, 0xBE, 0x71,
    0x3E, 0x48, 0x81, 0x3F, 0x64, 0x4F, 0x6A, 0x2B, 0xB9, 0x46, 0x7E, 0x22,
    0xB1, 0xE9, 0xA3, 0x82, 0xC3, 0xCA, 0x4B, 0xBA, 0x36, 0x61, 0x90, 0xB0,
    0x7A, 0x6E, 0x01, 0x01, 0xFC, 0x4B, 0xD8, 0xB0, 0xD5, 0x89, 0x5C, 0x5F,
    0x05, 0x4D, 0x6E, 0x27, 0xE3, 0x5E, 0x4B, 0xF2, 0x01, 0x04, 0xFB, 0xC9,
    0xD1, 0x66, 0x83, 0xCC, 0x7F, 0x3C, 0x3B, 0xAF, 0x49, 0x9A, 0x40, 0x31,
    0x96, 0x5F, 0xCB, 0x39, 0xD3, 0xB8, 0x8E, 0x4D, 0xCE, 0x90, 0xCC, 0x5E,
    0x8E, 0xDC, 0x0D, 0x8E, 0x76, 0x52, 0x88, 0xCA, 0xC3, 0xF7, 0xA3, 0x80,
    0xF6, 0xF5, 0x80, 0x32, 0xB7, 0x1E, 0x59, 0x54, 0xDF, 0x5F, 0x6C, 0xD1,
    0x5E, 0x04, 0x60, 0xFF, 0x24, 0xD4, 0x5D, 0x0B, 0x9A, 0x1C, 0xDD, 0x1E,
    0xD7, 0x2F, 0x25, 0x25, 0x88, 0xC1, 0x6C, 0xAD, 0x07, 0x09, 0x20, 0x7F,
    0x35, 0xD8, 0x27, 0x65, 0x5E, 0x79, 0xC2, 0xB9, 0x04, 0x1E, 0x87, 0x7B,
    0x7B, 0x8C, 0x3D
};

std::vector<std::string> Split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, delimiter)) {
        parts.push_back(item);
    }
    return parts;
}

std::vector<unsigned char> Base64UrlDecode(std::string text) {
    std::replace(text.begin(), text.end(), '-', '+');
    std::replace(text.begin(), text.end(), '_', '/');

    switch (text.size() % 4) {
    case 0:
        break;
    case 2:
        text += "==";
        break;
    case 3:
        text += '=';
        break;
    default:
        return {};
    }

    const DWORD output_length = static_cast<DWORD>((text.size() * 3) / 4);
    std::vector<unsigned char> output(output_length);
    DWORD decoded_length = output_length;

    if (!CryptStringToBinaryA(text.c_str(), static_cast<DWORD>(text.size()), CRYPT_STRING_BASE64,
                              output.data(), &decoded_length, nullptr, nullptr)) {
        return {};
    }

    output.resize(decoded_length);
    return output;
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

bool VerifySignature(const std::vector<unsigned char>& payload, const std::vector<unsigned char>& signature) {
    if (payload.empty() || signature.empty()) {
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
        || BCryptHashData(hash, const_cast<PUCHAR>(payload.data()),
                          static_cast<ULONG>(payload.size()), 0) != 0
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
    if (parts.size() != 3 || parts[0] != kPrefix) {
        return {false, "license token format is invalid"};
    }

    const auto payload_bytes = Base64UrlDecode(parts[1]);
    const auto signature = Base64UrlDecode(parts[2]);

    if (!VerifySignature(payload_bytes, signature)) {
        return {false, "license signature is invalid"};
    }

    nlohmann::json payload;
    try {
        payload = nlohmann::json::parse(payload_bytes.begin(), payload_bytes.end());
    } catch (...) {
        return {false, "license token format is invalid"};
    }

    const auto product = payload.value("product", "");
    if (product != kProduct) {
        return {false, "license product does not match Krit-Faslo"};
    }

    const auto version = payload.find("version");
    if (version == payload.end()
        || !((version->is_number_integer() && version->get<int>() == 1)
             || (version->is_string() && version->get<std::string>() == "1"))) {
        return {false, "license version is unsupported"};
    }

    const auto expires = payload.value("expires", "");
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
