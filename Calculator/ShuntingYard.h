#pragma once

#include <vector>
#include "Token.h"

class ShuntingYard {
public:
    std::vector<Token> toRpn(const std::vector<Token> &infix);
};
