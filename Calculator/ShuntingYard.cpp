#include <stack>
#include "ShuntingYard.h"
#include "OperatorTable.h"

std::vector<Token> ShuntingYard::toRpn(const std::vector<Token> &infix) {
    std::vector<Token> output;
    std::stack<Token> opStack;

    for (const auto &tok : infix) {
        if (tok.type == TokenType::Number) {
            output.push_back(tok);
        }
        else if (tok.type == TokenType::Operator) {
            const auto &op1 = OperatorTable::get(tok.text);

            while (!opStack.empty() &&
                   opStack.top().type == TokenType::Operator) {

                const auto &op2 =
                    OperatorTable::get(opStack.top().text);

                bool pop =
                    (!op1.rightAssociative && op1.precedence <= op2.precedence) ||
                    ( op1.rightAssociative && op1.precedence <  op2.precedence);

                if (!pop) break;

                output.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(tok);
        }
        else if (tok.type == TokenType::LeftBrac) {
            // 左括号：直接压栈
            opStack.push(tok);
        }
        else if (tok.type == TokenType::RightBrac) {
            // 右括号：弹到左括号
            while (!opStack.empty() &&
                   opStack.top().type != TokenType::LeftBrac) {
                output.push_back(opStack.top());
                opStack.pop();
            }

            // 丢弃左括号
            if (!opStack.empty() &&
                opStack.top().type == TokenType::LeftBrac) {
                opStack.pop();
            }
        }
    }

    while (!opStack.empty()) {
        output.push_back(opStack.top());
        opStack.pop();
    }

    return output;
}
