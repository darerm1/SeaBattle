#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>
#include <functional>
#include <memory>

class ClientNetwork : public QObject {
    Q_OBJECT

public:
    explicit ClientNetwork(QObject* parent = nullptr);
    ~ClientNetwork();

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();

    void sendCommand(const QString& command);

    bool isConnected() const { return socket_->state() == QTcpSocket::ConnectedState; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void loginResult(bool success, const QString& message);
    void signupResult(bool success, const QString& message);
    void shipPlaced(bool success, const QString& message);
    void moveResult(int result, int x, int y);  // result: 0=HIT, 1=MISS, 3=GAME_OVER
    void gameStarted();
    void opponentMove(int x, int y);
    void gameOver(int winnerId);
    void fieldCleared();
    void searchingOpponent();

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    void parseResponse(const QString& response);
    
    std::unique_ptr<QTcpSocket> socket_;
    QByteArray buffer_;
};
