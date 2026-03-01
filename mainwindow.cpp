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
        ui->stackedWidget->setCurrentWidget(ui->pageStart);
    });
    connect(ui->btnDebug, &QPushButton::clicked, this, [this]() {
        ui->pageDebug->pageClear();
        ui->stackedWidget->setCurrentWidget(ui->pageDebug);
    });
    connect(ui->btnRealtime, &QPushButton::clicked, this, [this]() {
        ui->pageRealtime->pageClear();
        ui->stackedWidget->setCurrentWidget(ui->pageRealtime);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
