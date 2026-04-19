#include "FeedbackWidget.h"
#include "ui_FeedbackWidget.h"
#include "Web/FeedbackClient.h"

#include <QFile>


FeedbackWidget::FeedbackWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FeedbackWidget)
{
    ui->setupUi(this);

    ui->Board->setEditable(false);

    connect(ui->btnBack, &QPushButton::clicked, this, [this]() {
        pageClear();
        emit btnBackPushed();
    });

    QFile file(":/style/feedback.css");
    if (file.open(QFile::ReadOnly)) {
        QString style = file.readAll();
        this->setStyleSheet(style);
    } else {
        qDebug() << "feedback.css 打不开";
    }

    ui->editAnalysis->setFrameShape(QFrame::NoFrame);
    ui->editAnalysis->viewport()->setAutoFillBackground(false);

    m_feedbackClient = new FeedbackClient(this);
    m_feedbackClient->setBaseUrl("http://127.0.0.1:8000");

    connect(m_feedbackClient, &FeedbackClient::submitSucceeded,
            this, [this](const FeedbackResult& result) {
                ui->editAnalysis->setText(result.analysisText);
                ui->editStatement->setText(QString("反馈提交成功，记录ID：%1").arg(result.reportId));
            });

    connect(m_feedbackClient, &FeedbackClient::submitFailed,
            this, [this](const QString& errorMessage) {
                ui->editStatement->setText("反馈提交失败");
                ui->editAnalysis->setText(errorMessage);
            });

    connect(ui->btnSend, &QPushButton::clicked, this, [this]() {
        if (m_debugData.canvas.isNull()) {
            ui->editStatement->setText("当前没有可提交的图像");
            return;
        }

        const QString expectedLabel = ui->boxCorrect->currentText().trimmed();
        const QString extraDescription = ui->editDesc->toPlainText().trimmed();

        if (expectedLabel.isEmpty()) {
            ui->editStatement->setText("请选择正确结果");
            return;
        }

        const QString predictedLabel = extractPredictedLabel(m_debugData.predictResult);
        const double confidenceValue = extractConfidenceValue(m_debugData.confidence);

        ui->editStatement->setText("正在提交反馈...");
        ui->editAnalysis->clear();

        // qDebug() << m_debugData.model << "\n"
        //          << predictedLabel << "\n"
        //          << confidenceValue << "\n"
        //          << expectedLabel << "\n"
        //          << extraDescription;

        m_feedbackClient->submitFeedback(
            m_debugData.canvas,
            m_debugData.model,
            predictedLabel,
            confidenceValue,
            expectedLabel,
            extraDescription
            );
    });
}

void FeedbackWidget::receiveData(DebugData& data) {
    m_debugData = data;

    ui->Board->setCanvas(data.canvas);
    ui->editModel->setText(data.model);
    ui->editPred->setText(data.predictResult);
    ui->editConf->setText(data.confidence);
}

void FeedbackWidget::pageClear() {
    ui->Board->clear();

    ui->editModel->clear();
    ui->editPred->clear();
    ui->editConf->clear();

    ui->editDesc->clear();
    ui->editAnalysis->clear();
    ui->editStatement->clear();
}

FeedbackWidget::~FeedbackWidget()
{
    delete ui;
}

QString FeedbackWidget::extractPredictedLabel(const QString& text) const {
    QString value = text.trimmed();
    int colonPos = value.indexOf(':');
    if (colonPos != -1) {
        value = value.mid(colonPos + 1);
    }
    return value.trimmed();
}

double FeedbackWidget::extractConfidenceValue(const QString& text) const {
    QString value = text.trimmed();
    int colonPos = value.indexOf(':');
    if (colonPos != -1) {
        value = value.mid(colonPos + 1);
    }

    value.remove('%');
    value = value.trimmed();

    bool ok = false;
    double number = value.toDouble(&ok);
    if (!ok) {
        return 0.0;
    }

    return number / 100.0;
}
