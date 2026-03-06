#pragma once
#include <string>

enum class TokenType {
    Number,
    Operator,
    LeftBrac,
    RightBrac
};

struct Token {
    TokenType type;
    std::string text;
};
