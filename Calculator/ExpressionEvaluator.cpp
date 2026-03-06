#include "ExpressionEvaluator.h"
#include "Tokenizer.h"
#include "ShuntingYard.h"
#include "RpnEvaluator.h"

BigDecimal ExpressionEvaluator::evaluate(const std::string &expr) {
    Tokenizer tokenizer;
    ShuntingYard shuntingYard;
    RpnEvaluator rpnEvaluator;

    auto tokens = tokenizer.tokenize(expr);
    auto rpn = shuntingYard.toRpn(tokens);

    return rpnEvaluator.evaluate(rpn);
}
