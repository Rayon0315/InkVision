#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QChart>
#include <QBarSeries>
#include <QBarSet>
#include <QChartView>
#include <QBarCategoryAxis>
#include <QValueAxis>

#include <opencv2/opencv.hpp>

namespace Ui {
class ChartWidget;
}

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

    void updateProb(const cv::Mat &prob);
    void clear();

private:
    Ui::ChartWidget *ui;

    QChart *chart;
    QBarSeries *series;
    QBarSet *set;
};

#endif // CHARTWIDGET_H
