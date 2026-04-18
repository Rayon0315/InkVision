#ifndef EXPRESSIONWIDGET_H
#define EXPRESSIONWIDGET_H

#include <QWidget>
#include "YOLO_Detector.h"
#include "Calculator/ExpressionEvaluator.h"
#include "Calculator/ExpressionValidator.h"

#include "Web/ExpressionRecognitionClient.h"

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

private slots:
    void onRecognizeExpressionClicked();
    void handleRecognitionSuccess(const ExpressionRecognitionResult &result);
    void handleRecognitionFailure(const QString &errorMessage);

private:
    Ui::ExpressionWidget *ui;

    YOLO_Detector detector;
    ExpressionValidator validator;
    ExpressionEvaluator evaluator;

    ExpressionRecognitionClient *m_expressionRecognitionClient = nullptr;
};

#endif // EXPRESSIONWIDGET_H
