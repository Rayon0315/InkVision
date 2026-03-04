#include "ChartWidget.h"
#include "ui_ChartWidget.h"

#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QBarCategoryAxis>

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
    QStringList categories;
    for (int i = 0; i < 10; i++)
        categories << QString::number(i);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
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

ChartWidget::~ChartWidget()
{
    delete ui;
}

void ChartWidget::updateProb(const cv::Mat &prob)
{
    if (prob.empty())
        return;

    set->remove(0, set->count());

    for (int i = 0; i < prob.cols; i++)
    {
        float p = prob.at<float>(0, i);
        *set << p;
    }
}
