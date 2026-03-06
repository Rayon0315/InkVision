#include <stack>

#include "RpnEvaluator.h"
#include "OperatorTable.h"

BigDecimal RpnEvaluator::evaluate(const std::vector<Token> &rpn) {
    std::stack<BigDecimal> st;

    for (const auto &tok : rpn) {
        if (tok.type == TokenType::Number) {
            st.push(BigDecimal::convertFromString(tok.text));
        } else {
            const auto  &info = OperatorTable::get(tok.text);

            if (info.arity == 1) {
                BigDecimal a = st.top(); st.pop();
                BigDecimal zero = BigDecimal::convertFromString("0");

                st.push(zero - a);
            } else {
                BigDecimal b = st.top(); st.pop();
                BigDecimal a = st.top(); st.pop();

                if (tok.text == "+") st.push(a + b);
                else if (tok.text == "-") st.push(a - b);
                else if (tok.text == "*") st.push(a * b);
                else if (tok.text == "/") st.push(a / b);
            }
        }
    }

    return st.top();
}
