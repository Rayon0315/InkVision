#include "mainwindow.h"
#include <QApplication>

#include <YOLO_Detector.h>

int main(int argc, char *argv[])
{
    YOLO_Detector det;
    det.basicTest();
    cv::Mat img = cv::imread("testImage/4.png");
    det.functionTest(img);
    std::cout << det.generateExpressionOri(img) << std::endl;

    QApplication a(argc, argv);
    MainWindow w;
    // w.show();
    return a.exec();
}
