#pragma once

#include <vector>
#include "Token.h"
#include "BigDecimal.h"

class RpnEvaluator {
public:
    BigDecimal evaluate(const std::vector<Token> &rpn);
};
