#ifndef EXPRESSIONWIDGET_H
#define EXPRESSIONWIDGET_H

#include <QWidget>
#include "YOLO_Detector.h"
#include "Calculator/ExpressionEvaluator.h"
#include "Calculator/ExpressionValidator.h"

namespace Ui {
class ExpressionWidget;
}

class ExpressionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExpressionWidget(QWidget *parent = nullptr);
    ~ExpressionWidget();

    void pageClear();

    QString detect();
    QString calculate(QString str);

private:
    Ui::ExpressionWidget *ui;

    YOLO_Detector detector;
    ExpressionValidator validator;
    ExpressionEvaluator evaluator;
};

#endif // EXPRESSIONWIDGET_H
