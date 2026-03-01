#pragma once

#include <QWidget>
#include <QImage>
#include <QPoint>

class DrawBoard : public QWidget {
    Q_OBJECT

private:
    QImage canvas;
    QPoint lastPos;

public:
    explicit DrawBoard(QWidget *parent = nullptr);

    void clear();
    std::vector<float> getNormalizedSize() const;
    std::vector<float> getNormalizedSizeMid() const;
    std::vector<float> getNormalizedSizeOri() const;

    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

signals:
    void mouseMoved(const QPoint& start, const QPoint& end); // 信号函数，供实时调用
};
