#ifndef REALTIMEWIDGET_H
#define REALTIMEWIDGET_H

#include <QWidget>
#include <opencv2/dnn.hpp>

namespace Ui {
class RealtimeWidget;
}

class RealtimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RealtimeWidget(QWidget *parent = nullptr);
    ~RealtimeWidget();

    void pageClear();
    void predictDigit();


private:
    Ui::RealtimeWidget *ui;

    cv::dnn::Net net;

private slots:
    void on_mouse_moved(const QPoint& start, const QPoint& end); // 槽函数，接收鼠标移动信号
};

#endif // REALTIMEWIDGET_H
