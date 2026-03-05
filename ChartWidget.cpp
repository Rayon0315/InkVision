#include "ChartWidget.h"
#include "ui_ChartWidget.h"

#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QBarCategoryAxis>

#include <algorithm>

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::ChartWidget)
{
    ui->setupUi(this);

    chart = new QChart();
    chart->setTitle("Digit Probability");

    series = new QBarSeries();
    set = new QBarSet("probability");

    // 初始化10个柱子
    for (int i = 0; i < 10; i++)
        *set << 0;

    series->append(set);
    chart->addSeries(series);

    // X轴
    axisX = new QBarCategoryAxis();
    QStringList categories;
    for (int i = 0; i < 10; i++)
        categories << QString::number(i);

    axisX->append(categories);

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Y轴
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 1);
    axisY->setTitleText("Probability");

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}

ChartWidget::~ChartWidget() {
    delete ui;
}

void ChartWidget::updateProb(const cv::Mat &prob) {
    if (prob.empty())
        return;

    set->remove(0, set->count());

    for (int i = 0; i < prob.cols; i++) {
        float p = prob.at<float>(0, i);
        *set << p;
    }
}

void ChartWidget::updateSortedProb(const cv::Mat &prob) {
    QVector<digitProb> data;
    for (int i = 0; i < prob.cols; i++) {
        data.push_back({i, prob.at<float>(0, i)});
    }
    std::sort(data.begin(), data.end(), [](const digitProb &x, const digitProb &y) {
        return x.prob > y.prob;
    });

    set->remove(0, set->count());
    QStringList categories;
    for (auto dat : data) {
        *set << dat.prob;
        categories << QString::number(dat.digit);
    }

    axisX->clear();
    axisX->append(categories);
}

void ChartWidget::clear() {
    set->remove(0, set->count());
}
