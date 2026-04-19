#include "DebugComplex.h"
#include "ui_DebugComplex.h"

DebugComplex::DebugComplex(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DebugComplex)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->pageDebug);

    connect(ui->pageDebug, &DebugWidget::btnFdbkPushed, this, [this](DebugData& data) {
        ui->stackedWidget->setCurrentWidget(ui->pageFeedback);
        ui->pageFeedback->receiveData(data);
    });

    connect(ui->pageFeedback, &FeedbackWidget::btnBackPushed, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageDebug);
    });
}

void DebugComplex::pageClear() {
    ui->stackedWidget->setCurrentWidget(ui->pageDebug);
    ui->pageDebug->pageClear();
}

DebugComplex::~DebugComplex()
{
    delete ui;
}
