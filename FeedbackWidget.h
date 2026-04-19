#ifndef FEEDBACKWIDGET_H
#define FEEDBACKWIDGET_H

#include <QWidget>

#include "DebugData.h"
#include "Web/FeedbackClient.h"

namespace Ui {
class FeedbackWidget;
}

class FeedbackWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FeedbackWidget(QWidget *parent = nullptr);
    ~FeedbackWidget();

    void receiveData(DebugData& data);

    void pageClear();

signals:
    void btnBackPushed();

private:
    Ui::FeedbackWidget *ui;

    FeedbackClient* m_feedbackClient = nullptr;
    DebugData m_debugData;

    QString extractPredictedLabel(const QString& text) const;
    double extractConfidenceValue(const QString& text) const;
};

#endif // FEEDBACKWIDGET_H
