#include "debugwindow.h"
#include "./ui_debugwindow.h"


DebugWindow::DebugWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DebugWindow)
{
    ui->setupUi(this);

    net = cv::dnn::readNetFromONNX("models/cnn.onnx");

    connect(ui->btnPred, &QPushButton::clicked, this, &DebugWindow::on_btnPred_clicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &DebugWindow::on_btnClear_clicked);
}

cv::Mat softmax(const cv::Mat& logits) {
    CV_Assert(logits.rows == 1);

    cv::Mat probs;
    logits.copyTo(probs);

    double maxVal;
    cv::minMaxLoc(probs, nullptr, &maxVal);
    probs -= maxVal;

    cv::exp(probs, probs);

    double sum = cv::sum(probs)[0];
    probs /= sum;

    return probs;
}

void DebugWindow::on_btnPred_clicked() {
    auto data = ui->Board->getNormalizedSize();

    cv::Mat input(1, 28 * 28, CV_32F, data.data());
    cv::Mat blob = input.reshape(1, {1, 1, 28, 28});

    net.setInput(blob);
    cv::Mat out = net.forward();
    cv::Mat prob = softmax(out);

    cv::Point classId;
    double conf;
    cv::minMaxLoc(prob, nullptr, &conf, nullptr, &classId);

    ui->editConf->setText(QString::fromStdString("Number: ") + QString::number(classId.x));
    ui->editPred_2->setText(QString::fromStdString("Confidence: ") + QString::number(conf * 100, 'f', 2) + "%");


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
}

void DebugWindow::on_btnClear_clicked() {
    ui->editConf->clear();
    ui->editPred_2->clear();
    ui->Board->clear();
}

DebugWindow::~DebugWindow()
{
    delete ui;
}
