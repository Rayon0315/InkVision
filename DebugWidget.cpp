#include "DebugWidget.h"
#include "ui_DebugWidget.h"
#include "MyMath.h"

DebugWidget::DebugWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DebugWidget)
{
    ui->setupUi(this);
    net = cv::dnn::readNetFromONNX("models/resnet.onnx");

    connect(ui->btnPred, &QPushButton::clicked, this, &DebugWidget::on_btnPred_clicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &DebugWidget::on_btnClear_clicked);
}

void DebugWidget::on_btnPred_clicked() {
    auto data = ui->Board->getNormalizedSize();

    cv::Mat input(1, 28 * 28, CV_32F, data.data());
    cv::Mat blob = input.reshape(1, {1, 1, 28, 28});

    net.setInput(blob);
    cv::Mat out = net.forward();
    cv::Mat prob = softmax(out);

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


    auto dataOri = ui->Board->getNormalizedSizeOri();
    cv::Mat mat28_1(28, 28, CV_32F, dataOri.data());
    cv::Mat mat28_copy_1 = mat28_1.clone();

    cv::Mat img8u_1;
    mat28_copy_1.convertTo(img8u_1, CV_8UC1, 255.);

    QImage img_1(img8u_1.data, 28, 28, img8u_1.step,
                 QImage::Format_Grayscale8);

    QPixmap pix_1 = QPixmap::fromImage(img_1)
                        .scaled(ui->labelOri->size(),
                                Qt::KeepAspectRatio,
                                Qt::FastTransformation);

    ui->labelOri->setPixmap(pix_1);


    auto dataMid = ui->Board->getNormalizedSizeMid();
    cv::Mat mat28_2(28, 28, CV_32F, dataMid.data());
    cv::Mat mat28_copy_2 = mat28_2.clone();

    cv::Mat img8u_2;
    mat28_copy_2.convertTo(img8u_2, CV_8UC1, 255.);

    QImage img_2(img8u_2.data, 28, 28, img8u_2.step,
                 QImage::Format_Grayscale8);

    QPixmap pix_2 = QPixmap::fromImage(img_2)
                        .scaled(ui->labelMid->size(),
                                Qt::KeepAspectRatio,
                                Qt::FastTransformation);

    ui->labelMid->setPixmap(pix_2);
}

void DebugWidget::on_btnClear_clicked() {
    ui->editConf->clear();
    ui->editPred->clear();
    ui->Board->clear();
    ui->labelOri->clear();
    ui->labelPix->clear();
    ui->labelMid->clear();
}

void DebugWidget::pageClear() {
    ui->Board->clear();
    ui->editConf->clear();
    ui->editPred->clear();
    ui->labelOri->clear();
    ui->labelPix->clear();
    ui->labelMid->clear();
}

DebugWidget::~DebugWidget()
{
    delete ui;
}
