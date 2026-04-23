#include "GestureWebSocketClient.h"

#include <QBuffer>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

GestureWebSocketClient::GestureWebSocketClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QWebSocket())
{
    m_socket->setParent(this);

    connect(m_socket, &QWebSocket::connected,
            this, &GestureWebSocketClient::onConnected);

    connect(m_socket, &QWebSocket::disconnected,
            this, &GestureWebSocketClient::onDisconnected);

    connect(m_socket, &QWebSocket::textMessageReceived,
            this, &GestureWebSocketClient::onTextMessageReceived);

    connect(m_socket, &QWebSocket::errorOccurred,
            this, &GestureWebSocketClient::onErrorOccurred);
}

void GestureWebSocketClient::setServerUrl(const QString& url)
{
    m_serverUrl = url;
}

QString GestureWebSocketClient::serverUrl() const
{
    return m_serverUrl;
}

void GestureWebSocketClient::connectToServer()
{
    if (m_serverUrl.isEmpty()) {
        emit socketError("WebSocket 服务器地址未设置");
        return;
    }

    if (m_socket->state() == QAbstractSocket::ConnectedState ||
        m_socket->state() == QAbstractSocket::ConnectingState) {
        return;
    }

    m_socket->open(QUrl(m_serverUrl));
}

void GestureWebSocketClient::disconnectFromServer()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState ||
        m_socket->state() == QAbstractSocket::ConnectingState) {
        m_socket->close();
    }
}

bool GestureWebSocketClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

QByteArray GestureWebSocketClient::imageToJpegBytes(const QImage& image, int quality) const
{
    QByteArray jpegBytes;
    QBuffer buffer(&jpegBytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG", quality);
    return jpegBytes;
}

void GestureWebSocketClient::resetState()
{
    if (!isConnected()) {
        emit resetFailed("WebSocket 未连接");
        return;
    }

    QJsonObject obj;
    obj["type"] = "reset";

    QJsonDocument doc(obj);
    m_socket->sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void GestureWebSocketClient::detectGesture(const QImage& roiImage)
{
    if (!isConnected()) {
        emit gestureFailed("WebSocket 未连接");
        return;
    }

    if (roiImage.isNull()) {
        emit gestureFailed("ROI 图像为空");
        return;
    }

    QByteArray jpegBytes = imageToJpegBytes(roiImage, 80);
    if (jpegBytes.isEmpty()) {
        emit gestureFailed("ROI 图像编码 JPG 失败");
        return;
    }

    QString imageBase64 = QString::fromUtf8(jpegBytes.toBase64());

    QJsonObject obj;
    obj["type"] = "frame";
    obj["image"] = imageBase64;

    QJsonDocument doc(obj);
    m_socket->sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void GestureWebSocketClient::onConnected()
{
    emit connected();
}

void GestureWebSocketClient::onDisconnected()
{
    emit disconnected();
}

void GestureWebSocketClient::onTextMessageReceived(const QString& message)
{
    handleJsonMessage(message.toUtf8());
}

void GestureWebSocketClient::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    emit socketError(m_socket->errorString());
}

void GestureWebSocketClient::handleJsonMessage(const QByteArray& jsonBytes)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit socketError("WebSocket 返回 JSON 解析失败");
        return;
    }

    QJsonObject root = doc.object();
    QString type = root.value("type").toString();

    if (type == "reset_ack") {
        int code = root.value("code").toInt(-1);
        QString message = root.value("message").toString();

        if (code == 0) {
            emit resetSucceeded();
        } else {
            emit resetFailed(message.isEmpty() ? "手势状态重置失败" : message);
        }
        return;
    }

    if (type == "gesture") {
        GestureResult result;
        result.code = root.value("code").toInt(-1);
        result.message = root.value("message").toString();
        result.success = (result.code == 0);

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
            emit gestureFailed(result.message.isEmpty() ? "手势识别失败" : result.message);
        }
        return;
    }

    if (type == "error") {
        QString message = root.value("message").toString();
        emit socketError(message.isEmpty() ? "WebSocket 服务端错误" : message);
        return;
    }

    emit socketError("未知的 WebSocket 消息类型");
}
