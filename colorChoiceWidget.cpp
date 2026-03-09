#include "colorChoiceWidget.h"
#include "ui_colorChoiceWidget.h"

#include <QFile>

#include "Common.h"

colorChoiceWidget::colorChoiceWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::colorChoiceWidget) {
    ui->setupUi(this);

    initColorCombo(ui->comboBox);
    setAttribute(Qt::WA_StyledBackground, true);

    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        QColor color = ui->comboBox->itemData(index).value<QColor>();
        emit colorChanged(color);
    });
}

void colorChoiceWidget::setLabelText(const QString& str) {
    ui->label->setText(str);
}

void colorChoiceWidget::setBoxChoice(const QString& color) {
    ui->comboBox->setCurrentText(color);
}

colorChoiceWidget::~colorChoiceWidget() {
    delete ui;
}
