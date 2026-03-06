#pragma once

#include <vector>
#include <string>

#include "Token.h"

enum class ValidationError {
    None,
    EmptyExpression,
    InvalidTokenSequence,
    MisMatchedParentheses,
    OperatorMisuse,
    OperandMissing,
    NotReducible
};

struct ValidationResult {
    bool ok;
    ValidationError error;
    int errorPos;
    std::string message;
};

typedef std::vector<Token>& TokenVecRef;
class Validator {
private:
    static ValidationResult checkEmpty(TokenVecRef tokens);
    static ValidationResult checkSequence(TokenVecRef tokens);
    static ValidationResult checkParentheses(TokenVecRef tokens);
    static ValidationResult checkOperatorSemantics(TokenVecRef tokens);
    static ValidationResult checkReducible(TokenVecRef tokens);

    static bool isValidTransition(TokenType prev, TokenType curr);
    static bool isUnaryPosition(TokenVecRef tokens, size_t i);
    static bool isRpnValid(TokenVecRef rpn);

public:
    static ValidationResult validate(TokenVecRef tokens);
};
