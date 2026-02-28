#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include <QWidget>
#include <opencv2/dnn.hpp>

namespace Ui {
class DebugWidget;
}

class DebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWidget(QWidget *parent = nullptr);
    ~DebugWidget();

    void on_btnPred_clicked();
    void on_btnClear_clicked();

    void pageClear();

private:
    Ui::DebugWidget *ui;

    cv::dnn::Net net;
};

#endif // DEBUGWIDGET_H
