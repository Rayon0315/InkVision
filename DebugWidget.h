#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include <QWidget>
#include <opencv2/dnn.hpp>

#include <DebugData.h>

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

signals:
    void btnFdbkPushed(DebugData& data);

private:
    Ui::DebugWidget *ui;
};

#endif // DEBUGWIDGET_H
