#pragma once

#include <QString>

struct FeedbackResult {
    bool success = false;

    int reportId = -1;
    QString analysisText;

    int code = -1;
    QString message;
};
