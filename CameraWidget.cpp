#include "CameraWidget.h"
#include "ui_CameraWidget.h"

#include <QMediaDevices>
#include <QCameraDevice>
#include <QCameraFormat>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QFile>

CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraWidget)
    , m_camera(nullptr)
    , m_captureSession(nullptr)
    , m_videoSink(nullptr)
    , m_gestureClient(nullptr)
    , m_gestureTimer(nullptr)
    , m_gesturePending(false)
    , m_cameraRunning(false)
    , m_mirrorEnabled(true)
    , m_showPinchIndicator(false)
    , m_trailMaxPoints(20)
{
    ui->setupUi(this);

    const QCameraDevice device = QMediaDevices::defaultVideoInput();
    if (device.isNull()) {
        qDebug() << "No camera device found.";
        return;
    }

    m_camera = new QCamera(device, this);

    for (const QCameraFormat& format : device.videoFormats()) {
        if (format.resolution() == QSize(1280, 960)) {
            m_camera->setCameraFormat(format);
            qDebug() << "Use 4:3 format:" << format.resolution();
            break;
        }
    }

    qDebug() << "Current camera format:" << m_camera->cameraFormat().resolution();

    m_captureSession = new QMediaCaptureSession(this);
    m_videoSink = new QVideoSink(this);

    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged,
            this, &CameraWidget::updateCurrentFrame);

    connect(m_camera, &QCamera::errorOccurred, this,
            [](QCamera::Error error, const QString& errorString) {
                qDebug() << "Camera error:" << error << errorString;
            });

    m_gestureClient = new GestureWebSocketClient(this);
    m_gestureClient->setServerUrl("ws://127.0.0.1:8000/ws/gesture");

    m_gestureTimer = new QTimer(this);
    m_gestureTimer->setInterval(20);

    connect(m_gestureTimer, &QTimer::timeout,
            this, &CameraWidget::sendGestureFrame);

    connect(m_gestureClient, &GestureWebSocketClient::connected,
            this, [this]() {
                qDebug() << "gesture websocket connected";
                m_gestureClient->resetState();
            });

    connect(m_gestureClient, &GestureWebSocketClient::disconnected,
            this, [this]() {
                qDebug() << "gesture websocket disconnected";
                m_gesturePending = false;
            });

    connect(m_gestureClient, &GestureWebSocketClient::resetSucceeded,
            this, [this]() {
                qDebug() << "gesture reset ok";
                m_gesturePending = false;
                m_gestureTimer->start();
            });

    connect(m_gestureClient, &GestureWebSocketClient::resetFailed,
            this, [this](const QString& err) {
                qDebug() << "gesture reset failed:" << err;
                m_gesturePending = false;
            });

    connect(m_gestureClient, &GestureWebSocketClient::gestureSucceeded,
            this, &CameraWidget::onGestureSucceeded);

    connect(m_gestureClient, &GestureWebSocketClient::gestureFailed,
            this, &CameraWidget::onGestureFailed);

    connect(m_gestureClient, &GestureWebSocketClient::socketError,
            this, [this](const QString& err) {
                qDebug() << "gesture websocket error:" << err;
                m_gesturePending = false;
            });

    connect(ui->Board, &DrawBoard::mouseMoved,
            this, &CameraWidget::onBoardStrokeUpdated);

    ui->Board->setModel(QString::fromStdString("ResNet"));

    connect(ui->btnClear, &QPushButton::clicked, this, [this]() {
        clear();
    });

    QFile file(":/style/basic.css");
    if (file.open(QFile::ReadOnly)) {
        QString style = file.readAll();
        this->setStyleSheet(style);
    } else {
        qDebug() << "basic.css 打不开";
    }
}

CameraWidget::~CameraWidget()
{
    delete ui;
}

void CameraWidget::updateCurrentFrame(const QVideoFrame &frame)
{
    if (!frame.isValid()) {
        return;
    }

    QVideoFrame copyFrame(frame);
    QImage image = copyFrame.toImage();

    if (image.isNull()) {
        return;
    }

    image = image.flipped(Qt::Horizontal);   // 水平镜像

    m_currentFrame = image;
    m_frameSize = image.size();
    update();
}

QImage CameraWidget::currentFrame() const
{
    return m_currentFrame.copy();
}

QRect CameraWidget::calcVideoDisplayRect() const
{
    QRect bounds = ui->horizontalLayout2->geometry();

    if (m_currentFrame.isNull() || !bounds.isValid()) {
        return bounds;
    }

    QSize scaled = m_currentFrame.size();
    scaled.scale(bounds.size(), Qt::KeepAspectRatio);

    int x = bounds.x() + (bounds.width() - scaled.width()) / 2;
    int y = bounds.y() + (bounds.height() - scaled.height()) / 2;

    return QRect(x, y, scaled.width(), scaled.height());
}

QRect CameraWidget::calcOverlayRoiRect() const
{
    QRect videoRect = calcVideoDisplayRect();
    if (!videoRect.isValid()) {
        return QRect();
    }

    int side = qMin(560, qMin(videoRect.width(), videoRect.height()) - 20);
    if (side <= 0) {
        return QRect();
    }

    int x = videoRect.x() + (videoRect.width() - side) / 2;
    int y = videoRect.y() + (videoRect.height() - side) / 2;

    return QRect(x, y, side, side);
}

QRect CameraWidget::roiRect() const
{
    return calcOverlayRoiRect();
}

QImage CameraWidget::currentRoiImage() const
{
    if (m_currentFrame.isNull()) {
        return QImage();
    }

    QRect videoRect = calcVideoDisplayRect();
    QRect overlayRect = calcOverlayRoiRect();

    if (!videoRect.isValid() || !overlayRect.isValid()) {
        return QImage();
    }

    double scaleX = static_cast<double>(m_currentFrame.width()) / videoRect.width();
    double scaleY = static_cast<double>(m_currentFrame.height()) / videoRect.height();

    int srcX = static_cast<int>((overlayRect.x() - videoRect.x()) * scaleX);
    int srcY = static_cast<int>((overlayRect.y() - videoRect.y()) * scaleY);
    int srcW = static_cast<int>(overlayRect.width() * scaleX);
    int srcH = static_cast<int>(overlayRect.height() * scaleY);

    QRect srcRect(srcX, srcY, srcW, srcH);
    srcRect = srcRect.intersected(m_currentFrame.rect());

    if (!srcRect.isValid()) {
        return QImage();
    }

    return m_currentFrame.copy(srcRect);
}

QSize CameraWidget::roiImageSize() const
{
    QImage roi = currentRoiImage();
    return roi.size();
}

void CameraWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect targetRect = ui->horizontalLayout2->geometry();

    painter.save();
    painter.setClipRect(targetRect);

    painter.fillRect(targetRect, QColor(0, 0, 0));

    if (!m_currentFrame.isNull()) {
        QRect videoRect = calcVideoDisplayRect();
        painter.drawImage(videoRect, m_currentFrame);
    }

    QRect roi = calcOverlayRoiRect();
    if (roi.isValid()) {
        QPen pen(QColor(110, 170, 255));
        pen.setWidth(3);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        painter.drawRect(roi);
    }

    // ===== 画 pinch 轨迹 =====
    if (!m_pinchTrail.isEmpty()) {
        for (int i = 0; i < m_pinchTrail.size(); ++i) {
            const QPoint& pt = m_pinchTrail[i];

            int alpha = 40 + (180 * (i + 1)) / m_pinchTrail.size();
            int radius = 3 + (4 * (i + 1)) / m_pinchTrail.size();

            QColor color(80, 200, 255, alpha);
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawEllipse(pt, radius, radius);
        }

        // 可选：再画一条连接线
        if (m_pinchTrail.size() >= 2) {
            QPen trailPen(QColor(80, 180, 255, 120));
            trailPen.setWidth(2);
            painter.setPen(trailPen);
            painter.setBrush(Qt::NoBrush);

            for (int i = 1; i < m_pinchTrail.size(); ++i) {
                painter.drawLine(m_pinchTrail[i - 1], m_pinchTrail[i]);
            }
        }
    }

    // ===== 画当前点 =====
    if (m_showPinchIndicator && !m_pinchTrail.isEmpty()) {
        QPoint current = m_pinchTrail.back();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 80, 120, 220));
        painter.drawEllipse(current, 8, 8);

        painter.setBrush(QColor(255, 80, 120, 80));
        painter.drawEllipse(current, 16, 16);
    }

    painter.restore();
}

void CameraWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void CameraWidget::startGestureLoop()
{
    m_gesturePending = false;
    if (m_gestureTimer->isActive()) {
        m_gestureTimer->stop();
    }
    m_gestureClient->resetState();
}

void CameraWidget::stopGestureLoop()
{
    if (m_gestureTimer->isActive()) {
        m_gestureTimer->stop();
    }
    m_gesturePending = false;
}

void CameraWidget::sendGestureFrame()
{
    if (!m_cameraRunning) {
        return;
    }

    if (!m_gestureClient->isConnected()) {
        return;
    }

    if (m_gesturePending) {
        return;
    }

    QImage roiImage = currentRoiImage();
    if (roiImage.isNull()) {
        return;
    }

    m_gesturePending = true;
    m_gestureClient->detectGesture(roiImage);
}

QPoint CameraWidget::mapGesturePointToBoard(const QPoint& roiPoint) const
{
    QSize roiSize = roiImageSize();
    QSize boardSize = ui->Board->size();   // 替换成你的对象名

    if (roiSize.width() <= 0 || roiSize.height() <= 0) {
        return QPoint();
    }

    return QPoint(
        roiPoint.x() * boardSize.width() / roiSize.width(),
        roiPoint.y() * boardSize.height() / roiSize.height()
        );
}

void CameraWidget::onGestureSucceeded(const GestureResult& result)
{
    m_gesturePending = false;

    if (result.strokeEvent == "start" && result.hasPoint) {
        QPoint boardPoint = mapGesturePointToBoard(result.point);
        ui->Board->beginAirStroke(boardPoint);

        m_pinchTrail.clear();
        m_pinchTrail.push_back(mapGesturePointToPreview(result.point));
        m_showPinchIndicator = true;
    }
    else if (result.strokeEvent == "move" && result.hasPoint) {
        QPoint boardPoint = mapGesturePointToBoard(result.point);
        ui->Board->appendAirStrokePoint(boardPoint);

        QPoint previewPoint = mapGesturePointToPreview(result.point);
        m_pinchTrail.push_back(previewPoint);

        while (m_pinchTrail.size() > m_trailMaxPoints) {
            m_pinchTrail.pop_front();
        }

        m_showPinchIndicator = true;
    }
    else if (result.strokeEvent == "end") {
        ui->Board->endAirStroke();

        m_showPinchIndicator = false;
        m_pinchTrail.clear();   // 第一个选择：立即清空轨迹
    }

    update();
}

void CameraWidget::onGestureFailed(const QString& err)
{
    m_gesturePending = false;
    qDebug() << "gesture failed:" << err;
}

void CameraWidget::onBoardStrokeUpdated(const QPoint& start, const QPoint& end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);

    cv::Mat prob = ui->Board->predict();
    ui->Chart->updateProb(prob);
}

void CameraWidget::startCamera()
{
    if (!m_camera || m_cameraRunning) {
        return;
    }

    m_camera->start();
    m_cameraRunning = true;

    m_gestureClient->connectToServer();
}

void CameraWidget::stopCamera()
{
    if (!m_camera || !m_cameraRunning) {
        return;
    }

    stopGestureLoop();
    m_gestureClient->disconnectFromServer();

    m_camera->stop();
    m_cameraRunning = false;
    m_gesturePending = false;
    m_currentFrame = QImage();
    m_frameSize = QSize();

    if (ui->Board->isAirDrawing()) {
        ui->Board->endAirStroke();
    }

    update();
}

bool CameraWidget::isCameraRunning() const
{
    return m_cameraRunning;
}

void CameraWidget::clear() {
    ui->Board->clear();
    ui->Chart->clear();
}

QPoint CameraWidget::mapGesturePointToPreview(const QPoint& roiPoint) const
{
    QRect roiDisplayRect = roiRect();     // 这是界面上显示的 ROI 框
    QSize roiSize = roiImageSize();       // 这是发给服务端的 ROI 图像大小

    if (!roiDisplayRect.isValid() || roiSize.width() <= 0 || roiSize.height() <= 0) {
        return QPoint();
    }

    int x = roiDisplayRect.x() + roiPoint.x() * roiDisplayRect.width() / roiSize.width();
    int y = roiDisplayRect.y() + roiPoint.y() * roiDisplayRect.height() / roiSize.height();

    return QPoint(x, y);
}
