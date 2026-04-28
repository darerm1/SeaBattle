#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>

class NetworkWorker : public QObject {
    Q_OBJECT

public:
    explicit NetworkWorker(QObject* parent = nullptr);

public slots:
    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    void sendCommand(const QString& command);

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

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    void parseResponse(const QString& response);

    QTcpSocket* socket_;
    QByteArray buffer_;
};
