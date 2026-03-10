#include "CustomizedWidget.h"
#include "ui_CustomizedWidget.h"

#include <QFile>

CustomizedWidget::CustomizedWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomizedWidget) {
    ui->setupUi(this);

    QFile file(":/style/basic.css");
    if (file.open(QFile::ReadOnly)) {
        QString style = file.readAll();
        this->setStyleSheet(style);
    } else {
        qDebug() << "style.css 打不开";
    }

    ui->btnEraser->setCheckable(true);
    ui->btnEraser->setIconSize(QSize(22, 22));
    ui->btnEraser->setFixedHeight(36);
    ui->btnEraser->setIcon(QIcon(":/icons/pencil.svg"));
    ui->btnEraser->setText("画笔");
    connect(ui->btnEraser, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            ui->btnEraser->setIcon(QIcon(":/icons/eraser.svg"));
            ui->btnEraser->setText("橡皮");
            ui->Board->toEraser();
        } else {
            ui->btnEraser->setIcon(QIcon(":/icons/pencil.svg"));
            ui->btnEraser->setText("画笔");
            ui->Board->fromEraser();
        }
    });

    ui->colorBackground->setLabelText("画布颜色");
    ui->colorBackground->setBoxChoice("Black");

    ui->colorPen->setLabelText("画笔颜色");
    ui->colorPen->setBoxChoice("White");

    connect(ui->colorBackground, &colorChoiceWidget::colorChanged, this, [this](const QColor& color) {
        ui->Board->setBackgroundColor(color);
        ui->editConf->clear();
        ui->editPred->clear();
        ui->Board->clear();
        ui->labelPix->clear();
    });
    connect(ui->colorPen, &colorChoiceWidget::colorChanged, this, [this](const QColor& color) {
        ui->Board->setPenColor(color);
        ui->editConf->clear();
        ui->editPred->clear();
        ui->Board->clear();
        ui->labelPix->clear();
    });

    connect(ui->btnPred, &QPushButton::clicked, this, &CustomizedWidget::on_btnPred_clicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &CustomizedWidget::on_btnClear_clicked);

    connect(ui->slidePenWidth, &QSlider::valueChanged, this, [this](int value) {
        ui->Board->setPenWidth(value);
    });
}

void CustomizedWidget::pageClear() {
    ui->editConf->clear();
    ui->editPred->clear();
    ui->Board->clear();
    ui->labelPix->clear();
}

void CustomizedWidget::on_btnPred_clicked() {
    auto data = ui->Board->getNormalizedSize();

    // cv::Mat input(1, 28 * 28, CV_32F, data.data());
    // cv::Mat blob = input.reshape(1, {1, 1, 28, 28});

    // net.setInput(blob);
    // cv::Mat out = net.forward();
    // cv::Mat prob = softmax(out);

    ui->Board->setModel(ui->BoxModelChoice->currentText());
    cv::Mat prob = ui->Board->predict();

    cv::Point classId;
    double conf;
    cv::minMaxLoc(prob, nullptr, &conf, nullptr, &classId);

    ui->editPred->setText(QString::fromStdString("Number: ") + QString::number(classId.x));
    ui->editConf->setText(QString::fromStdString("Confidence: ") + QString::number(conf * 100, 'f', 2) + "%");


    // 调试：输出28×28图像
    cv::Mat mat28(28, 28, CV_32F, data.data());
    cv::Mat mat28_copy = mat28.clone();

    cv::Mat img8u;
    mat28_copy.convertTo(img8u, CV_8UC1, 255.);

    QImage img(img8u.data, 28, 28, img8u.step,
               QImage::Format_Grayscale8);

    QPixmap pix = QPixmap::fromImage(img)
                      .scaled(ui->labelPix->size(),
                              Qt::KeepAspectRatio,
                              Qt::FastTransformation);

    ui->labelPix->setPixmap(pix);
}

void CustomizedWidget::on_btnClear_clicked() {
    ui->editConf->clear();
    ui->editPred->clear();
    ui->Board->clear();
    ui->labelPix->clear();
}

CustomizedWidget::~CustomizedWidget() {
    delete ui;
}
