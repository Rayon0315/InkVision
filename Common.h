#ifndef COMMON_H
#define COMMON_H

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <qcombobox.h>

cv::Mat QImageToMat(const QImage& img);

QIcon makeColorIcon(const QColor& color);

void initColorCombo(QComboBox* box);

Qt::GlobalColor toGlobalColor(const QColor& c);

#endif // COMMON_H
