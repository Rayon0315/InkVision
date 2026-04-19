#ifndef DEBUGDATA_H
#define DEBUGDATA_H

#include <QImage>
#include <QString>

struct DebugData {
    QImage canvas;

    QString model;
    QString predictResult;
    QString confidence;
};

#endif // DEBUGDATA_H
