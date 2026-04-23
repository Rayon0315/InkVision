#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include <QTimer>
#include <QVector>

#include "Web/GestureWebSocketClient.h"

namespace Ui {
class CameraWidget;
}

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();

    QImage currentFrame() const;
    QRect roiRect() const;
    QImage currentRoiImage() const;
    QSize roiImageSize() const;

    void startCamera();
    void stopCamera();
    bool isCameraRunning() const;
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QRect calcVideoDisplayRect() const;
    QRect calcOverlayRoiRect() const;
    void updateCurrentFrame(const QVideoFrame &frame);

    void startGestureLoop();
    void stopGestureLoop();
    void sendGestureFrame();


    QPoint mapGesturePointToBoard(const QPoint& roiPoint) const;

    void onGestureSucceeded(const GestureResult& result);
    void onGestureFailed(const QString& err);
    void onBoardStrokeUpdated(const QPoint& start, const QPoint& end);

    QPoint mapGesturePointToPreview(const QPoint& roiPoint) const;

private:
    Ui::CameraWidget *ui;

    QCamera* m_camera;
    QMediaCaptureSession* m_captureSession;
    QVideoSink* m_videoSink;

    QImage m_currentFrame;
    QSize m_frameSize;

    GestureWebSocketClient* m_gestureClient;
    QTimer* m_gestureTimer;
    bool m_gesturePending;

    bool m_cameraRunning;
    bool m_mirrorEnabled;

    QVector<QPoint> m_pinchTrail;     // 新增：pinch轨迹点
    bool m_showPinchIndicator;        // 新增：是否显示当前位置
    int m_trailMaxPoints;             // 新增：轨迹最多保留多少点
};

#endif // CAMERAWIDGET_H
