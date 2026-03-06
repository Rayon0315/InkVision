#pragma once

#include "Validator.h"
#include <string>

class ExpressionValidator {
public:
    ValidationResult validate(const std::string &expr);
};
