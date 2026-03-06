#pragma once
#include <string>
#include <unordered_map>

struct OperatorInfo {
    int precedence;           // 优先级
    bool rightAssociative;    // 是否右关联
    int arity;                // 运算符目数
};

class OperatorTable {
private:
    static const std::unordered_map<std::string, OperatorInfo> table;

public:
    static const OperatorInfo& get(const std::string &op);
    static bool isOperator(const std::string &op);
};
