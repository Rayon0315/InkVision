#ifndef GESTURERESULT_H
#define GESTURERESULT_H

#include <QString>
#include <QPoint>

struct GestureResult {
    bool success = false;
    int code = -1;
    QString message;

    bool handDetected = false;
    bool pinchActive = false;
    QString strokeEvent;   // start / move / end / idle

    bool hasPoint = false;
    QPoint point;

    double pinchDistance = 0.0;
};

#endif // GESTURERESULT_H
