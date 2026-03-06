#include <cctype>
#include "Tokenizer.h"

std::vector<Token> Tokenizer::tokenize(const std::string &expr) {
    std::vector<Token> tokens;
    size_t i = 0;

    auto prevIsOperatorOrLeftBrac = [&]() {
        if (tokens.empty()) return true;

        auto t = tokens.back().type;
        return t == TokenType::Operator || t == TokenType::LeftBrac;
    };

    while (i < expr.size()) {
        char c = expr[i];

        if (std::isspace(c)) {
            i++;
            continue;
        }

        if (std::isdigit(c) || c == '.') {
            size_t start = i;

            while (i < expr.size() && (std::isdigit(expr[i]) || expr[i] == '.')) {
                i++;
            }

            tokens.push_back({TokenType::Number, expr.substr(start, i - start)});

            continue;
        }

        if (c == '(') {
            tokens.push_back({TokenType::LeftBrac, "("});
            i++;
            continue;
        }

        if (c == ')') {
            tokens.push_back({TokenType::RightBrac, ")"});
            i++;
            continue;
        }

        if (c == '-' && prevIsOperatorOrLeftBrac()) {
            tokens.push_back({TokenType::Operator, "NEG"});
        } else {
            tokens.push_back({TokenType::Operator, std::string(1, c)});
        }

        i++;
    }

    return tokens;
}
