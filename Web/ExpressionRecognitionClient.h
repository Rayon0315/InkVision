#ifndef EXPRESSIONRECOGNITIONCLIENT_H
#define EXPRESSIONRECOGNITIONCLIENT_H

#include <QObject>
#include <QImage>
#include <QHash>
#include <QNetworkAccessManager>

#include "ExpressionRecognitionResult.h"

class QNetworkReply;
class QTimer;

class ExpressionRecognitionClient : public QObject {
    Q_OBJECT

public:
    ExpressionRecognitionClient(QObject* parent = nullptr);

    void setBaseUrl(const QString& baseUrl);
    QString baseUrl() const;

    void setRequestTimeout(int ms);
    int requestTimeout() const;

    void recognizeExpression(const QImage &image);

signals:
    void recognizeSucceeded(const ExpressionRecognitionResult &result);
    void recognizeFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QByteArray imageToPngBytes(const QImage &image) const;
    void cleanupReply(QNetworkReply *reply);

    QNetworkAccessManager *m_manager = nullptr;
    QString m_baseUrl;

    int m_requestTimeoutMs = 15000;
    QHash<QNetworkReply*, QTimer*> m_replyTimers;
};

#endif // EXPRESSIONRECOGNITIONCLIENT_H
