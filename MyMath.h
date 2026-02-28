#ifndef MYMATH_H
#define MYMATH_H

#include <opencv2/dnn.hpp>

cv::Mat softmax(const cv::Mat& logits);

#endif // MYMATH_H
