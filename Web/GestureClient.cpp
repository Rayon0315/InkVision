#include "GestureClient.h"

#include <QBuffer>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>
#include <QNetworkRequest>
#include <qnetworkaccessmanager.h>

GestureClient::GestureClient(QObject* parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &GestureClient::onReplyFinished);
}

void GestureClient::setBaseUrl(const QString& baseUrl)
{
    m_baseUrl = baseUrl;
    if (m_baseUrl.endsWith('/')) {
        m_baseUrl.chop(1);
    }
}

QString GestureClient::baseUrl() const
{
    return m_baseUrl;
}

void GestureClient::setRequestTimeout(int ms)
{
    m_requestTimeoutMs = ms;
}

int GestureClient::requestTimeout() const
{
    return m_requestTimeoutMs;
}

QByteArray GestureClient::imageToPngBytes(const QImage& image) const
{
    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return pngBytes;
}

void GestureClient::resetState()
{
    if (m_baseUrl.isEmpty()) {
        emit resetFailed("服务器地址未设置");
        return;
    }

    QUrl url(m_baseUrl + "/api/v1/gesture/reset");
    QNetworkRequest request(url);

    QNetworkReply* reply = m_manager->post(request, QByteArray());

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, [reply]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    });

    m_replyTimers.insert(reply, timer);
    timer->start(m_requestTimeoutMs);
}

void GestureClient::detectGesture(const QImage& roiImage)
{
    if (m_baseUrl.isEmpty()) {
        emit gestureFailed("服务器地址未设置");
        return;
    }

    if (roiImage.isNull()) {
        emit gestureFailed("ROI 图像为空");
        return;
    }

    QByteArray pngBytes = imageToPngBytes(roiImage);
    if (pngBytes.isEmpty()) {
        emit gestureFailed("ROI 图像编码 PNG 失败");
        return;
    }

    QUrl url(m_baseUrl + "/api/v1/gesture/pinch");
    QNetworkRequest request(url);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"image\"; filename=\"roi.png\""));
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("image/png"));

    QBuffer* buffer = new QBuffer(multiPart);
    buffer->setData(pngBytes);
    buffer->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(buffer);
    multiPart->append(imagePart);

    QNetworkReply* reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply);

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, [reply]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    });

    m_replyTimers.insert(reply, timer);
    timer->start(m_requestTimeoutMs);
}

void GestureClient::onReplyFinished(QNetworkReply* reply)
{
    if (!reply) {
        emit gestureFailed("无效网络响应");
        return;
    }

    QByteArray responseBytes = reply->readAll();
    QUrl url = reply->request().url();
    QString path = url.path();

    cleanupReply(reply);

    if (reply->error() != QNetworkReply::NoError) {
        QString errorText = reply->errorString();
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            errorText = "请求超时或已取消";
        }

        if (path.endsWith("/reset")) {
            emit resetFailed(errorText);
        } else {
            emit gestureFailed(errorText);
        }

        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (path.endsWith("/reset")) {
            emit resetFailed("服务器返回 JSON 解析失败");
        } else {
            emit gestureFailed("服务器返回 JSON 解析失败");
        }

        reply->deleteLater();
        return;
    }

    QJsonObject root = doc.object();
    int code = root.value("code").toInt(-1);
    QString message = root.value("message").toString();

    if (path.endsWith("/reset")) {
        if (code == 0) {
            emit resetSucceeded();
        } else {
            emit resetFailed(message.isEmpty() ? "重置手势状态失败" : message);
        }

        reply->deleteLater();
        return;
    }

    GestureResult result;
    result.code = code;
    result.message = message;
    result.success = (code == 0);

    QJsonValue dataValue = root.value("data");
    if (dataValue.isObject()) {
        QJsonObject dataObj = dataValue.toObject();

        result.handDetected = dataObj.value("hand_detected").toBool(false);
        result.pinchActive = dataObj.value("pinch_active").toBool(false);
        result.strokeEvent = dataObj.value("stroke_event").toString();
        result.pinchDistance = dataObj.value("pinch_distance").toDouble(0.0);

        QJsonValue pointValue = dataObj.value("point");
        if (pointValue.isObject()) {
            QJsonObject pointObj = pointValue.toObject();
            result.hasPoint = true;
            result.point = QPoint(
                pointObj.value("x").toInt(),
                pointObj.value("y").toInt()
                );
        }
    }

    if (result.success) {
        emit gestureSucceeded(result);
    } else {
        QString err = result.message;
        if (err.isEmpty()) {
            err = "手势识别失败";
        }
        emit gestureFailed(err);
    }

    reply->deleteLater();
}

void GestureClient::cleanupReply(QNetworkReply* reply)
{
    if (!m_replyTimers.contains(reply)) {
        return;
    }

    QTimer* timer = m_replyTimers.take(reply);
    if (timer) {
        timer->stop();
        timer->deleteLater();
    }
}
