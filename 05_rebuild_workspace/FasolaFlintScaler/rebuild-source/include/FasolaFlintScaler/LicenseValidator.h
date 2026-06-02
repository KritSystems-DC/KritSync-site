#pragma once

#include <string>
#include <vector>

namespace FasolaFlintScaler {

struct LicenseResult {
    bool valid = false;
    std::string message;
};

class LicenseValidator {
public:
    LicenseResult Validate(const std::string& token) const;

private:
    static bool ConstantTimeEqual(const std::vector<unsigned char>& left,
                                  const std::vector<unsigned char>& right);
};

}  // namespace FasolaFlintScaler
