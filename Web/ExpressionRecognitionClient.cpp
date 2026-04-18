#include "ExpressionRecognitionClient.h"

#include <QBuffer>
#include <QFile>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

ExpressionRecognitionClient::ExpressionRecognitionClient(QObject* parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)) {
    connect(m_manager, &QNetworkAccessManager::finished, this, &ExpressionRecognitionClient::onReplyFinished);
}

void ExpressionRecognitionClient::setBaseUrl(const QString& baseUrl) {
    m_baseUrl = baseUrl;
    if (m_baseUrl.endsWith('/')) {
        m_baseUrl.chop(1);
    }
}

QString ExpressionRecognitionClient::baseUrl() const {
    return m_baseUrl;
}

void ExpressionRecognitionClient::setRequestTimeout(int ms) {
    m_requestTimeoutMs = ms;
}

QByteArray ExpressionRecognitionClient::imageToPngBytes(const QImage& image) const {
    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);

    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return pngBytes;
}

void ExpressionRecognitionClient::recognizeExpression(const QImage &image) {
    if (m_baseUrl.isEmpty()) {
        emit recognizeFailed("服务器地址未设置");
        return;
    }

    if (image.isNull()) {
        emit recognizeFailed("图像为空，无法发送识别请求");
        return;
    }

    QByteArray pngBytes = imageToPngBytes(image);
    if (pngBytes.isEmpty()) {
        emit recognizeFailed("图像编码 PNG 失败");
        return;
    }

    QUrl url(m_baseUrl + "/api/v1/expression/recognize");
    QNetworkRequest request(url);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"file\"; filename=\"expression.png\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                       QVariant("image/png"));

    QBuffer *buffer = new QBuffer(multiPart);
    buffer->setData(pngBytes);
    buffer->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(buffer);

    multiPart->append(filePart);

    QNetworkReply *reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply); // reply 销毁时一并清理 multipart

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, [reply]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    });

    m_replyTimers.insert(reply, timer);
    timer->start(m_requestTimeoutMs);
}

void ExpressionRecognitionClient::onReplyFinished(QNetworkReply *reply) {
    if (!reply) {
        emit recognizeFailed("无效网络响应");
        return;
    }

    QByteArray responseBytes = reply->readAll();
    cleanupReply(reply);

    if (reply->error() != QNetworkReply::NoError) {
        QString errorText = reply->errorString();
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            errorText = "请求超时或已取消";
        }
        emit recognizeFailed(errorText);
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit recognizeFailed("服务器返回 JSON 解析失败");
        reply->deleteLater();
        return;
    }

    QJsonObject root = doc.object();

    ExpressionRecognitionResult result;
    result.code = root.value("code").toInt(-1);
    result.message = root.value("message").toString();

    QJsonValue dataValue = root.value("data");
    if (dataValue.isObject()) {
        QJsonObject dataObj = dataValue.toObject();
        result.expression = dataObj.value("expression").toString();
        result.result = dataObj.value("result").toString();
        result.error = dataObj.value("error").toString();
    }

    // code == 0 表示接口层成功
    // 业务层如果识别失败，通常会体现在 data.error 非空
    if (result.code == 0) {
        if (!result.error.isEmpty()) {
            result.success = false;
            emit recognizeFailed(result.error);
        } else {
            result.success = true;
            emit recognizeSucceeded(result);
        }
    } else {
        QString err = result.message;
        if (err.isEmpty()) {
            err = "接口调用失败";
        }
        emit recognizeFailed(err);
    }

    reply->deleteLater();
}

void ExpressionRecognitionClient::cleanupReply(QNetworkReply *reply) {
    if (!m_replyTimers.contains(reply)) {
        return;
    }

    QTimer *timer = m_replyTimers.take(reply);
    if (timer) {
        timer->stop();
        timer->deleteLater();
    }
}
