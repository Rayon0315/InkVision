#include "ExpressionWidget.h"
#include "ui_ExpressionWidget.h"

#include <QFile>
#include <opencv2/opencv.hpp>

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
    });
    connect(ui->btnClear, &QPushButton::clicked, this, [this]() {
        pageClear();
    });

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
}

ExpressionWidget::~ExpressionWidget() {
    delete ui;
}
