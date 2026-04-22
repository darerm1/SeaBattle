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

    virtual void sendCommand(const QString& command);

    bool isConnected() const { return socket_->state() == QTcpSocket::ConnectedState; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void loginResult(bool success, const QString& message);
    void signupResult(bool success, const QString& message);
    void shipPlaced(bool success, const QString& message);
    void moveResult(int result, int x, int y);
    void opponentFound();
    void gameStarted();
    void yourTurn();
    void gameForfeited();
    void gameInfo(QString ownLogin, int ownRating, QString oppLogin, int oppRating);
    void opponentMove(int x, int y, int result);
    void gameOver(int winnerId);
    void fieldCleared();
    void searchingOpponent();
    void ratingUpdated(int rating);
    void opponentLeft();

protected:
    void parseResponse(const QString& response);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    
    std::unique_ptr<QTcpSocket> socket_;
    QByteArray buffer_;
};
