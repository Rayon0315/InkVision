#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    setStyleSheet("QMainWindow { background-color: #1F2228; }");

    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->pageStart);

    connect(ui->btnStart, &QPushButton::clicked, this, [this]() {
        if (ui->stackedWidget->currentWidget() == ui->pageCamera) {
            ui->pageCamera->stopCamera();
        }
        ui->stackedWidget->setCurrentWidget(ui->pageStart);
    });
    connect(ui->btnDebug, &QPushButton::clicked, this, [this]() {
        if (ui->stackedWidget->currentWidget() == ui->pageCamera) {
            ui->pageCamera->stopCamera();
        }
        ui->pageDebug->pageClear();
        ui->stackedWidget->setCurrentWidget(ui->pageDebug);
    });
    connect(ui->btnRealtime, &QPushButton::clicked, this, [this]() {
        if (ui->stackedWidget->currentWidget() == ui->pageCamera) {
            ui->pageCamera->stopCamera();
        }
        ui->pageRealtime->pageClear();
        ui->stackedWidget->setCurrentWidget(ui->pageRealtime);
    });
    connect(ui->btnExpr, &QPushButton::clicked, this, [this]() {
        if (ui->stackedWidget->currentWidget() == ui->pageCamera) {
            ui->pageCamera->stopCamera();
        }
        ui->pageExpr->pageClear();
        ui->stackedWidget->setCurrentWidget(ui->pageExpr);
    });
    connect(ui->btnCustomized, &QPushButton::clicked, this, [this]() {
        if (ui->stackedWidget->currentWidget() == ui->pageCamera) {
            ui->pageCamera->stopCamera();
        }
        ui->pageCustomized->pageClear();
        ui->stackedWidget->setCurrentWidget(ui->pageCustomized);
    });
    connect(ui->btnCamera, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageCamera);
        ui->pageCamera->startCamera();
        ui->pageCamera->clear();
    });

    QPixmap pix(":/images/logo.jpg");
    ui->icon->setPixmap(pix);
    ui->icon->setScaledContents(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}
