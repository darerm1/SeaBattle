#pragma once

#include <QObject>
#include <QThread>
#include <QString>
#include "network/network_worker.hpp"

class ClientNetwork : public QObject {
    Q_OBJECT

public:
    explicit ClientNetwork(QObject* parent = nullptr);
    ~ClientNetwork();

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    virtual void sendCommand(const QString& command);
    bool isConnected() const { return connected_; }

signals:
    void connected();
    void disconnected();
    void connectRequested(QString host, quint16 port);
    void disconnectRequested();
    void commandRequested(QString command);
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

private:
    NetworkWorker* networkWorker_;
    QThread networkThread_;
    bool connected_ = false;
};
