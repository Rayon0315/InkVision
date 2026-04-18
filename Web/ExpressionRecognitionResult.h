#pragma once

#include <QString>

struct ExpressionRecognitionResult {
    bool success = false;

    QString expression;
    QString result;
    QString error;

    int code = -1;
    QString message;
};
