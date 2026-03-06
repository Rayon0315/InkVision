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
    std::string result = detector.generateExpression(img);
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
    ui->Board->clear(Qt::white);
    ui->editExpr->clear();
    ui->editResult->clear();
}

ExpressionWidget::~ExpressionWidget() {
    delete ui;
}
