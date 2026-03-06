#include "ExpressionValidator.h"
#include "Tokenizer.h"
#include "Validator.h"

ValidationResult ExpressionValidator::validate(const std::string &expr) {
    Tokenizer tokenizer;

    auto tokens = tokenizer.tokenize(expr);
    auto result = Validator::validate(tokens);

    return result;
}
