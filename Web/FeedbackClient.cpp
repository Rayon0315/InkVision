#include "FeedbackClient.h"

#include <QBuffer>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

FeedbackClient::FeedbackClient(QObject* parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)) {
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &FeedbackClient::onReplyFinished);
}

void FeedbackClient::setBaseUrl(const QString& baseUrl) {
    m_baseUrl = baseUrl;
    if (m_baseUrl.endsWith('/')) {
        m_baseUrl.chop(1);
    }
}

QString FeedbackClient::baseUrl() const {
    return m_baseUrl;
}

void FeedbackClient::setRequestTimeout(int ms) {
    m_requestTimeoutMs = ms;
}

int FeedbackClient::requestTimeout() const {
    return m_requestTimeoutMs;
}

QByteArray FeedbackClient::imageToPngBytes(const QImage& image) const {
    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return pngBytes;
}

void FeedbackClient::submitFeedback(
    const QImage& image,
    const QString& modelName,
    const QString& predictedLabel,
    double confidence,
    const QString& expectedLabel,
    const QString& extraDescription
    ) {
    if (m_baseUrl.isEmpty()) {
        emit submitFailed("服务器地址未设置");
        return;
    }

    if (image.isNull()) {
        emit submitFailed("图像为空，无法发送反馈请求");
        return;
    }

    QByteArray pngBytes = imageToPngBytes(image);
    if (pngBytes.isEmpty()) {
        emit submitFailed("图像编码 PNG 失败");
        return;
    }

    QUrl url(m_baseUrl + "/api/v1/feedback/report");
    QNetworkRequest request(url);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 1. image
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"image\"; filename=\"feedback.png\""));
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("image/png"));

    QBuffer* buffer = new QBuffer(multiPart);
    buffer->setData(pngBytes);
    buffer->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(buffer);
    multiPart->append(imagePart);

    // 2. model_name
    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"model_name\""));
    modelPart.setBody(modelName.toUtf8());
    multiPart->append(modelPart);

    // 3. predicted_label
    QHttpPart predictedPart;
    predictedPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant("form-data; name=\"predicted_label\""));
    predictedPart.setBody(predictedLabel.toUtf8());
    multiPart->append(predictedPart);

    // 4. confidence
    QHttpPart confidencePart;
    confidencePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                             QVariant("form-data; name=\"confidence\""));
    confidencePart.setBody(QString::number(confidence, 'f', 4).toUtf8());
    multiPart->append(confidencePart);

    // 5. expected_label
    QHttpPart expectedPart;
    expectedPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant("form-data; name=\"expected_label\""));
    expectedPart.setBody(expectedLabel.toUtf8());
    multiPart->append(expectedPart);

    // 6. extra_description
    QHttpPart descPart;
    descPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"extra_description\""));
    descPart.setBody(extraDescription.toUtf8());
    multiPart->append(descPart);

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

void FeedbackClient::onReplyFinished(QNetworkReply* reply) {
    if (!reply) {
        emit submitFailed("无效网络响应");
        return;
    }

    QByteArray responseBytes = reply->readAll();
    cleanupReply(reply);

    if (reply->error() != QNetworkReply::NoError) {
        QString errorText = reply->errorString();
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            errorText = "请求超时或已取消";
        }
        emit submitFailed(errorText);
        reply->deleteLater();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit submitFailed("服务器返回 JSON 解析失败");
        reply->deleteLater();
        return;
    }

    QJsonObject root = doc.object();

    FeedbackResult result;
    result.code = root.value("code").toInt(-1);
    result.message = root.value("message").toString();

    QJsonValue dataValue = root.value("data");
    if (dataValue.isObject()) {
        QJsonObject dataObj = dataValue.toObject();
        result.reportId = dataObj.value("report_id").toInt(-1);
        result.analysisText = dataObj.value("analysis_text").toString();
    }

    if (result.code == 0) {
        result.success = true;
        emit submitSucceeded(result);
    } else {
        QString err = result.message;
        if (err.isEmpty()) {
            err = "反馈提交失败";
        }
        emit submitFailed(err);
    }

    reply->deleteLater();
}

void FeedbackClient::cleanupReply(QNetworkReply* reply) {
    if (!m_replyTimers.contains(reply)) {
        return;
    }

    QTimer* timer = m_replyTimers.take(reply);
    if (timer) {
        timer->stop();
        timer->deleteLater();
    }
}
