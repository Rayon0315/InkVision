#ifndef FEEDBACKCLIENT_H
#define FEEDBACKCLIENT_H

#include <QObject>
#include <QImage>
#include <QHash>
#include <QNetworkAccessManager>

#include "FeedbackResult.h"

class QNetworkReply;
class QTimer;

class FeedbackClient : public QObject {
    Q_OBJECT

public:
    explicit FeedbackClient(QObject* parent = nullptr);

    void setBaseUrl(const QString& baseUrl);
    QString baseUrl() const;

    void setRequestTimeout(int ms);
    int requestTimeout() const;

    void submitFeedback(
        const QImage& image,
        const QString& modelName,
        const QString& predictedLabel,
        double confidence,
        const QString& expectedLabel,
        const QString& extraDescription
        );

signals:
    void submitSucceeded(const FeedbackResult& result);
    void submitFailed(const QString& errorMessage);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QByteArray imageToPngBytes(const QImage& image) const;
    void cleanupReply(QNetworkReply* reply);

    QNetworkAccessManager* m_manager = nullptr;
    QString m_baseUrl;

    int m_requestTimeoutMs = 15000;
    QHash<QNetworkReply*, QTimer*> m_replyTimers;
};

#endif // FEEDBACKCLIENT_H
