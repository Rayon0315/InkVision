#include "MyMath.h"

cv::Mat softmax(const cv::Mat& logits) {
    CV_Assert(logits.rows == 1);

    cv::Mat probs;
    logits.copyTo(probs);

    double maxVal;
    cv::minMaxLoc(probs, nullptr, &maxVal);
    probs -= maxVal;

    cv::exp(probs, probs);

    double sum = cv::sum(probs)[0];
    probs /= sum;

    return probs;
}
