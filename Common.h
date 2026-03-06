#ifndef COMMON_H
#define COMMON_H

#include <QWidget>
#include <opencv2/opencv.hpp>

cv::Mat QImageToMat(const QImage& img);

#endif // COMMON_H
