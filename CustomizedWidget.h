#ifndef CUSTOMIZEDWIDGET_H
#define CUSTOMIZEDWIDGET_H

#include <QWidget>

namespace Ui {
class CustomizedWidget;
}

class CustomizedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomizedWidget(QWidget *parent = nullptr);
    ~CustomizedWidget();

    void pageClear();

    void on_btnPred_clicked();
    void on_btnClear_clicked();

private:
    Ui::CustomizedWidget *ui;
};

#endif // CUSTOMIZEDWIDGET_H
