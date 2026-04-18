#include "ExpressionWidget.h"
#include "ui_ExpressionWidget.h"

#include <QFile>
#include <opencv2/opencv.hpp>
#include <Common.h>

ExpressionWidget::ExpressionWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExpressionWidget) {
    ui->setupUi(this);

    ui->Board->adjustSize(640, 256);
    ui->Board->setPenColor(Qt::black);
    ui->Board->setPenWidth(15);
    ui->Board->setBackgroundColor(Qt::white);

    connect(ui->btnDetect, &QPushButton::clicked, this, [this]() {
        QString str = detect();
        ui->editExpr->setText("Expression: " + str);
        ui->editResult->setText(calculate(str));

        // cv::imshow("show", QImageToMat(ui->Board->exportProcessedImage()));
    });
    connect(ui->btnClear, &QPushButton::clicked, this, [this]() {
        pageClear();
    });

    m_expressionRecognitionClient = new ExpressionRecognitionClient(this);

    m_expressionRecognitionClient->setBaseUrl("http://127.0.0.1:8000");
    m_expressionRecognitionClient->setRequestTimeout(15000);

    connect(ui->btnAi, &QPushButton::clicked,
            this, &ExpressionWidget::onRecognizeExpressionClicked);

    connect(m_expressionRecognitionClient, &ExpressionRecognitionClient::recognizeSucceeded,
            this, &ExpressionWidget::handleRecognitionSuccess);

    connect(m_expressionRecognitionClient, &ExpressionRecognitionClient::recognizeFailed,
            this, &ExpressionWidget::handleRecognitionFailure);

    QFile file(":/style/expr.css");
    if (file.open(QFile::ReadOnly)) {
        QString style = file.readAll();
        this->setStyleSheet(style);
    } else {
        qDebug() << "style.css 打不开";
    }
}

QString ExpressionWidget::detect() {
    cv::Mat img = ui->Board->getCanvasMat();


    int target_width = 320, target_height = 128;
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(target_width, target_height));

    cv::Mat padded;

    int target = 960;

    int top = (target - img.rows) / 2;
    int bottom = target - img.rows - top;

    int left = (target - img.cols) / 2;
    int right = target - img.cols - left;

    cv::copyMakeBorder(
        resized,
        padded,
        top,
        bottom,
        left,
        right,
        cv::BORDER_CONSTANT,
        cv::Scalar(255,255,255)   // 白色
        );

    std::string result = detector.generateExpression(padded);
    return QString::fromStdString(result);
}

QString ExpressionWidget::calculate(QString str) {
    ValidationResult validResult = validator.validate(str.toStdString());

    if (validResult.ok != true) {
        return "Calculation Failed: " + QString::fromStdString(validResult.message);
    }

    BigDecimal resultData = evaluator.evaluate(str.toStdString());
    QString result = "Calculation Succeeded: " + QString::fromStdString(resultData.convertToString());

    return result;
}

void ExpressionWidget::pageClear() {
    ui->Board->clear();
    ui->editExpr->clear();
    ui->editResult->clear();
    ui->editAi->clear();
    ui->editStatement->clear();
}

ExpressionWidget::~ExpressionWidget() {
    delete ui;
}

void ExpressionWidget::onRecognizeExpressionClicked() {
    QImage image = ui->Board->exportProcessedImage();

    if (image.isNull()) {
        ui->editStatement->setText("没有可识别内容");
        return;
    }

    ui->btnAi->setEnabled(false);
    ui->editStatement->setText("正在请求服务器...");
    ui->editAi->clear();

    m_expressionRecognitionClient->recognizeExpression(image);
}

void ExpressionWidget::handleRecognitionSuccess(const ExpressionRecognitionResult &result) {
    ui->btnAi->setEnabled(true);
    ui->editStatement->setText("识别成功");

    QString displayText;
    if (!result.expression.isEmpty() && !result.result.isEmpty()) {
        displayText = result.expression + " = " + result.result;
    } else if (!result.expression.isEmpty()) {
        displayText = "表达式：" + result.expression;
    } else {
        displayText = "返回内容为空";
    }

    ui->editAi->setText(displayText);
}

void ExpressionWidget::handleRecognitionFailure(const QString &errorMessage) {
    ui->btnAi->setEnabled(true);
    ui->editStatement->setText("识别失败");
    ui->editAi->setText(errorMessage);
}
