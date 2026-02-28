#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QMainWindow>
#include <opencv2/dnn.hpp>

namespace Ui {
class DebugWindow;
}

class DebugWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = nullptr);
    ~DebugWindow();

    void on_btnPred_clicked();
    void on_btnClear_clicked();

private:
    Ui::DebugWindow *ui;

    cv::dnn::Net net;
};

#endif // DEBUGWINDOW_H
