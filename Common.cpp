#include "Common.h"
#include <qpainter.h>

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

QIcon makeColorIcon(const QColor& color) {
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setBrush(color);
    p.setPen(Qt::black);
    p.drawRect(0, 0, 15, 15);

    return QIcon(pix);
}


void initColorCombo(QComboBox* box) {
    box->addItem(makeColorIcon(Qt::black), "Black", QColor(Qt::black));
    box->addItem(makeColorIcon(QColor(255,107,107)), "Red",   QColor(255,107,107));
    box->addItem(makeColorIcon(QColor(77,163,255)),  "Blue",  QColor(77,163,255));
    box->addItem(makeColorIcon(QColor(76,217,100)),  "Green", QColor(76,217,100));
    box->addItem(makeColorIcon(Qt::white), "White", QColor(Qt::white));
}

Qt::GlobalColor toGlobalColor(const QColor& c) {
    if (c.rgb() == QColor(Qt::black).rgb()) return Qt::black;
    if (c.rgb() == QColor(Qt::white).rgb()) return Qt::white;
    if (c.rgb() == QColor(Qt::red).rgb())   return Qt::red;
    if (c.rgb() == QColor(Qt::green).rgb()) return Qt::green;
    if (c.rgb() == QColor(Qt::blue).rgb())  return Qt::blue;

    return Qt::black; // fallback
}
