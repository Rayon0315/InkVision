#ifndef GESTURECLIENT_H
#define GESTURECLIENT_H

#include <QObject>
#include <QImage>
#include <QHash>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "GestureResult.h"

class GestureClient : public QObject
{
    Q_OBJECT

public:
    explicit GestureClient(QObject* parent = nullptr);

    void setBaseUrl(const QString& baseUrl);
    QString baseUrl() const;

    void setRequestTimeout(int ms);
    int requestTimeout() const;

    void resetState();
    void detectGesture(const QImage& roiImage);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QByteArray imageToPngBytes(const QImage& image) const;
    void cleanupReply(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_manager;
    QString m_baseUrl;
    int m_requestTimeoutMs = 5000;
    QHash<QNetworkReply*, QTimer*> m_replyTimers;

signals:
    void resetSucceeded();
    void resetFailed(const QString& errorMessage);

    void gestureSucceeded(const GestureResult& result);
    void gestureFailed(const QString& errorMessage);
};

#endif // GESTURECLIENT_H
