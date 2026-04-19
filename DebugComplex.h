#ifndef DEBUGCOMPLEX_H
#define DEBUGCOMPLEX_H

#include <QWidget>

namespace Ui {
class DebugComplex;
}

class DebugComplex : public QWidget
{
    Q_OBJECT

public:
    explicit DebugComplex(QWidget *parent = nullptr);
    ~DebugComplex();

    void pageClear();

private:
    Ui::DebugComplex *ui;
};

#endif // DEBUGCOMPLEX_H
