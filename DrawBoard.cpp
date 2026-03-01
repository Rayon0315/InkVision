#include "DrawBoard.h"
#include <QPainter>
#include <QMouseEvent>

#include <opencv2/opencv.hpp>

DrawBoard::DrawBoard(QWidget *parent)
    : QWidget(parent),
    canvas(280, 280, QImage::Format_Grayscale8) {
    canvas.fill(Qt::black);
    setFixedSize(280, 280);
}

void DrawBoard::clear() {
    canvas.fill(Qt::black);
    update();
}

void DrawBoard::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawImage(0, 0, canvas);
}

void DrawBoard::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton)
        lastPos = e->pos();
}

void DrawBoard::mouseMoveEvent(QMouseEvent *e) {
    if (!(e->buttons() & Qt::LeftButton)) return;

    QPainter p(&canvas);
    p.setPen(QPen(Qt::white, 25, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(lastPos, e->pos());
    lastPos = e->pos();
    update();

    emit mouseMoved(lastPos, e->pos());
}

// 粗暴缩放：正确率不高
std::vector<float> DrawBoard::getNormalizedSizeOri() const {
    QImage small = canvas.scaled(28, 28, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    std::vector<float> data(28 * 28);
    for (int y = 0; y < 28; y++) {
        for (int x = 0; x < 28; x++) {
            int gray = qGray(small.pixel(x, y));
            data[y * 28 + x] = gray / 255.f;
        }
    }

    return data;
}


// 利用opencv进行调整，但是缩放策略有问题
std::vector<float> DrawBoard::getNormalizedSizeMid() const {
    // 用cv::Mat处理数据
    cv::Mat img(canvas.height(), canvas.width(), CV_8UC1,
                const_cast<uchar*>(canvas.bits()),
                canvas.bytesPerLine());

    // 防止浅拷贝
    img = img.clone();

    // 二值化
    cv::threshold(img, img, 10, 255, cv::THRESH_BINARY);

    // 查看前景区域
    std::vector<cv::Point> points;
    cv::findNonZero(img, points);

    if (points.empty()) return std::vector<float>(28 * 28, 0.f);

    cv::Rect bbox = cv::boundingRect(points);
    cv::Mat digit = img(bbox);

    // 缩放到 20×20
    cv::Mat resized;
    cv::resize(digit, resized, cv::Size(20, 20), 0, 0, cv::INTER_AREA);

    // 居中填充到 28×28
    cv::Mat padded = cv::Mat::zeros(28, 28, CV_32F);

    cv::Mat resized_f;
    resized.convertTo(resized_f, CV_32F, 1. / 255.);

    resized_f.copyTo(padded(cv::Rect(4, 4, 20, 20)));

    // 高斯模糊：模仿MNIST
    cv::GaussianBlur(padded, padded, cv::Size(3, 3), 0.5);

    std::vector<float> data(28 * 28);
    for (int y = 0; y < 28; y++) {
        for (int x = 0; x < 28; x++) {
            data[y * 28 + x] = padded.at<float>(y, x);
        }
    }

    return data;
}

std::vector<float> DrawBoard::getNormalizedSize() const {

    // === 1. 将 QImage 转为 cv::Mat ===
    cv::Mat img(canvas.height(),
                canvas.width(),
                CV_8UC1,
                const_cast<uchar*>(canvas.bits()),
                canvas.bytesPerLine());

    img = img.clone();  // 防止浅拷贝问题

    // === 2. 二值化 ===
    cv::threshold(img, img, 10, 255, cv::THRESH_BINARY);

    // === 3. 查找前景区域 ===
    std::vector<cv::Point> points;
    cv::findNonZero(img, points);

    if (points.empty())
        return std::vector<float>(28 * 28, 0.f);

    cv::Rect bbox = cv::boundingRect(points);
    cv::Mat digit = img(bbox);

    // === 4. 等比例缩放，使最长边 = 20 ===
    int w = digit.cols;
    int h = digit.rows;

    int new_w, new_h;

    if (w > h) {
        new_w = 20;
        new_h = static_cast<int>(h * 20.0 / w);
    } else {
        new_h = 20;
        new_w = static_cast<int>(w * 20.0 / h);
    }

    cv::Mat resized;
    cv::resize(digit, resized, cv::Size(new_w, new_h), 0, 0, cv::INTER_AREA);

    // === 5. 转 float 并归一化 ===
    cv::Mat resized_f;
    resized.convertTo(resized_f, CV_32F, 1.0 / 255.0);

    // === 6. 放入 28×28 中心 ===
    cv::Mat padded = cv::Mat::zeros(28, 28, CV_32F);

    int x_offset = (28 - new_w) / 2;
    int y_offset = (28 - new_h) / 2;

    resized_f.copyTo(
        padded(cv::Rect(x_offset, y_offset, new_w, new_h))
        );

    // === 7. 质心对齐（更接近 MNIST 处理方式）=== （工程上不好用）
    // cv::Moments m = cv::moments(padded, true);

    // if (m.m00 > 1e-6) {
    //     int cx = static_cast<int>(m.m10 / m.m00);
    //     int cy = static_cast<int>(m.m01 / m.m00);

    //     int shift_x = 14 - cx;
    //     int shift_y = 14 - cy;

    //     cv::Mat M = (cv::Mat_<double>(2, 3) <<
    //                      1, 0, shift_x,
    //                  0, 1, shift_y);

    //     cv::warpAffine(padded, padded, M, padded.size(),
    //                    cv::INTER_LINEAR,
    //                    cv::BORDER_CONSTANT,
    //                    cv::Scalar(0));
    // }

    // === 8. 高斯模糊（模拟 MNIST 模糊感）===
    cv::GaussianBlur(padded, padded, cv::Size(3, 3), 0.5);

    // === 9. 输出为一维向量 ===
    std::vector<float> data(28 * 28);

    for (int y = 0; y < 28; ++y) {
        for (int x = 0; x < 28; ++x) {
            data[y * 28 + x] = padded.at<float>(y, x);
        }
    }

    return data;
}
