#include "RealtimeWidget.h"
#include "ui_RealtimeWidget.h"
#include "MyMath.h"

RealtimeWidget::RealtimeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RealtimeWidget)
{
    ui->setupUi(this);

    net = cv::dnn::readNetFromONNX("models/resnet.onnx");

    connect(ui->btnClear, &QPushButton::clicked, this, [this]() {
        pageClear();
    });

    connect(ui->Board, &DrawBoard::mouseMoved, this, &RealtimeWidget::on_mouse_moved);
}

void RealtimeWidget::pageClear() {
    ui->Board->clear();
    ui->editConf->clear();
    ui->editPred->clear();
    ui->labelPix->clear();
}

void RealtimeWidget::predictDigit() {
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
}

void RealtimeWidget::on_mouse_moved(const QPoint& start, const QPoint& end) {
    predictDigit();
}


RealtimeWidget::~RealtimeWidget()
{
    delete ui;
}
