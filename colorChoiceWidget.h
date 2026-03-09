#ifndef COLORCHOICEWIDGET_H
#define COLORCHOICEWIDGET_H

#include <QWidget>

namespace Ui {
class colorChoiceWidget;
}

class colorChoiceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit colorChoiceWidget(QWidget *parent = nullptr);
    ~colorChoiceWidget();

    void setLabelText(const QString& str);
    void setBoxChoice(const QString& color);

signals:
    void colorChanged(const QColor& color);

private:
    Ui::colorChoiceWidget *ui;
};

#endif // COLORCHOICEWIDGET_H
