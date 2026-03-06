#pragma once

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <opencv2/dnn.hpp>



class DrawBoard : public QWidget {
    Q_OBJECT

private:
    QImage canvas;
    QPoint lastPos;
    int penWidth;
    Qt::GlobalColor penColor;
    cv::dnn::Net net;
    QMap<QString, QString> model;
    int width, height;

public:
    explicit DrawBoard(
        QWidget *parent = nullptr,
        int width = 280,
        int height = 280
    );

    void adjustSize(int width, int height);

    void clear(Qt::GlobalColor color = Qt::black);
    std::vector<float> getNormalizedSize() const;
    std::vector<float> getNormalizedSizeMid() const;
    std::vector<float> getNormalizedSizeOri() const;

    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

    void setPenWidth(int value);
    void setPenColor(Qt::GlobalColor color = Qt::white);

    void setModel(QString modelName);
    cv::Mat predict();

    cv::Mat getCanvasMat();

signals:
    void mouseMoved(const QPoint& start, const QPoint& end); // 信号函数，供实时调用
};
