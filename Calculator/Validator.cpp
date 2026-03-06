#include "Validator.h"

#include "OperatorTable.h"
#include "ShuntingYard.h"
#include <functional>
using std::vector;
using std::function;

#define OK { true, ValidationError::None, -1, "" };

// 统一合法性接口
ValidationResult Validator::validate(TokenVecRef tokens) {
    ValidationResult r;

    vector<function<ValidationResult(TokenVecRef)>> checkFuncs = {
        checkEmpty,
        checkSequence,
        checkParentheses,
        checkOperatorSemantics,
        checkReducible
    };

    for (auto func : checkFuncs) {
        r = func(tokens);
        if (!r.ok) return r;
    }

    return OK;
}

// 空表达式
ValidationResult Validator::checkEmpty(TokenVecRef tokens) {
    if (tokens.empty())
        return { false, ValidationError::EmptyExpression, -1, "Expression is Empty" };

    return OK;
}

// 相邻Token合法性
bool Validator::isValidTransition(TokenType prev, TokenType curr) {
    using T = TokenType;

    if (prev == T::Number && (curr == T::Number || curr == T::LeftBrac))
        return false;
    if (prev == T::Operator && (curr == T::Operator || curr == T::RightBrac))
        return false;
    if (prev == T::LeftBrac && curr == T::RightBrac)
        return false;
    if (prev == T::RightBrac && (curr == T::Number || curr == T::LeftBrac))
        return false;

    return true;
}

ValidationResult Validator::checkSequence(TokenVecRef tokens) {
    TokenType t;

    // 起始Token合法性
    t = tokens.front().type;
    if (!(t == TokenType::Number||
          t == TokenType::LeftBrac ||
          t == TokenType::Operator)) {
        return { false, ValidationError::InvalidTokenSequence, 0, "Invalid start of expression" };
    }

    // 结尾Token合法性
    t = tokens.back().type;
    if (!(t == TokenType::Number ||
          t == TokenType::RightBrac)) {
        return { false, ValidationError::InvalidTokenSequence, (int)tokens.size() - 1, "Invalid end of expression" };
    }

    // 中间相邻Token合法性
    for (size_t i = 1; i < tokens.size(); i++) {
        if (!isValidTransition(tokens[i - 1].type, tokens[i].type)) {
            return { false, ValidationError::InvalidTokenSequence, (int)i, "Invalid token sequence" };
        }
    }

    return OK;
}

// 括号匹配
ValidationResult Validator::checkParentheses(TokenVecRef tokens) {
    int balance = 0;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type == TokenType::LeftBrac)
            balance++;
        else if (tokens[i].type == TokenType::RightBrac)
            balance--;

        if (balance < 0)
            return { false, ValidationError::MisMatchedParentheses, (int)i, "Extra ')'" };
    }

    if (balance != 0)
        return { false, ValidationError::MisMatchedParentheses, -1, "Unmatched '('"};

    return OK;
}

// 运算符语义
bool Validator::isUnaryPosition(TokenVecRef tokens, size_t i) {
    if (i == 0) return true;

    auto prev = tokens[i - 1].type;
    return prev == TokenType::Operator || prev == TokenType::LeftBrac;
}

ValidationResult Validator::checkOperatorSemantics(TokenVecRef tokens) {
    using T = TokenType;

    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type != T::Operator) continue;

        bool unary = isUnaryPosition(tokens, i);

        if (unary) {
            if (i + 1 >= tokens.size())
                return { false, ValidationError::OperandMissing, (int)i, "Missing operand after unary operator" };

            auto next = tokens[i + 1].type;
            if (!(next == T::Number || next == T::LeftBrac))
                return { false, ValidationError::OperatorMisuse, (int)i, "Invalid unary operator usage" };
        } else {
            if (i + 1 >= tokens.size())
                return { false, ValidationError::OperandMissing, (int)i, "Missing right operand" };
            auto next = tokens[i + 1].type;
            if (!(next == T::Number || next == T::LeftBrac))
                return { false, ValidationError::OperandMissing, (int)i, "Invalid right operand" };
        }
    }

    return OK;
}

// RPN可归约性
bool Validator::isRpnValid(TokenVecRef rpn) {
    int stack = 0;

    for (auto &tok : rpn) {
        if (tok.type == TokenType::Number) {
            stack++;
        } else if (tok.type == TokenType::Operator) {
            OperatorInfo op = OperatorTable::get(tok.text);
            int need = op.arity;

            if (stack < need) return false;

            stack -= need;
            stack++;
        }
    }

    return stack == 1;
}

ValidationResult Validator::checkReducible(TokenVecRef tokens) {
    try {
        ShuntingYard SY;
        auto rpn = SY.toRpn(tokens);

        if (!isRpnValid(rpn))
            return { false, ValidationError::NotReducible, -1, "Expression is not reducible" };
    } catch (...) {
        return { false, ValidationError::NotReducible, -1, "Expression is not reducible" };
    }

    return OK;
}
