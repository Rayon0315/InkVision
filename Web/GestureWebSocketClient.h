#ifndef GESTUREWEBSOCKETCLIENT_H
#define GESTUREWEBSOCKETCLIENT_H

#include <QObject>
#include <QImage>
#include <QUrl>
#include <QWebSocket>

#include "GestureResult.h"

class GestureWebSocketClient : public QObject
{
    Q_OBJECT

public:
    explicit GestureWebSocketClient(QObject* parent = nullptr);

    void setServerUrl(const QString& url);
    QString serverUrl() const;

    void connectToServer();
    void disconnectFromServer();

    bool isConnected() const;

    void resetState();
    void detectGesture(const QImage& roiImage);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onErrorOccurred(QAbstractSocket::SocketError error);

private:
    QByteArray imageToJpegBytes(const QImage& image, int quality = 80) const;
    void handleJsonMessage(const QByteArray& jsonBytes);

private:
    QWebSocket* m_socket;
    QString m_serverUrl;

signals:
    void connected();
    void disconnected();

    void resetSucceeded();
    void resetFailed(const QString& errorMessage);

    void gestureSucceeded(const GestureResult& result);
    void gestureFailed(const QString& errorMessage);

    void socketError(const QString& errorMessage);
};

#endif // GESTUREWEBSOCKETCLIENT_H
