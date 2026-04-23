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
    QColor penColor;
    QColor backgroundColor;
    enum DrawMode {
        PEN,
        ERASER
    } mode;

    cv::dnn::Net net;
    QMap<QString, QString> model;
    int width, height;

    bool editable;

    bool airDrawing;

    void drawSegment(const QPoint& start, const QPoint& end); // 统一绘制一段线
    QPoint clampPoint(const QPoint& pt) const;                // 坐标裁剪

public:
    explicit DrawBoard(
        QWidget *parent = nullptr,
        int width = 280,
        int height = 280
    );

    void adjustSize(int width, int height);

    void clear();
    std::vector<float> getNormalizedSize() const;
    std::vector<float> getNormalizedSizeMid() const;
    std::vector<float> getNormalizedSizeOri() const;

    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

    void setPenWidth(int value);
    void setPenColor(QColor color = Qt::white);
    void setBackgroundColor(QColor color = Qt::black);

    void setModel(QString modelName);
    cv::Mat predict();

    cv::Mat getCanvasMat();


    void toEraser();
    void fromEraser();

    QImage getCanvas();
    QImage exportProcessedImage();

    void setEditable(bool ok);
    void setCanvas(QImage& inputCanvas);

    // 供摄像头/手势输入使用
    void beginAirStroke(const QPoint& pt);
    void appendAirStrokePoint(const QPoint& pt);
    void endAirStroke();
    bool isAirDrawing() const;

signals:
    void mouseMoved(const QPoint& start, const QPoint& end); // 信号函数，供实时调用
};
