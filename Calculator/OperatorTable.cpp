#include "OperatorTable.h"

const std::unordered_map<std::string, OperatorInfo> OperatorTable::table = {
    {"+", { 1, false, 2 }},
    {"-", { 1, false, 2 }},
    {"*", { 2, false, 2 }},
    {"/", { 2, false, 2 }},
    {"NEG", { 3, true, 1 }}
};

const OperatorInfo& OperatorTable::get(const std::string &op) {
    return table.at(op);
}

bool OperatorTable::isOperator(const std::string &op) {
    return table.count(op) > 0;
}
