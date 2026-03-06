#include "Common.h"

cv::Mat QImageToMat(const QImage& img) {
    QImage rgb = img.convertToFormat(QImage::Format_RGB888);

    return cv::Mat(
               rgb.height(),
               rgb.width(),
               CV_8UC3,
               (void*)rgb.constBits(),
               rgb.bytesPerLine()
               ).clone();
}
