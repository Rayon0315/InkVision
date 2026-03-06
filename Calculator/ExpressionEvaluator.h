#pragma once

#include <string>
#include "BigDecimal.h"

class ExpressionEvaluator {
public:
    BigDecimal evaluate(const std::string &expr);
};
