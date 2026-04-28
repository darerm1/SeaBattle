#include "network/network_worker.hpp"
#include <QHostAddress>
#include <QDebug>

NetworkWorker::NetworkWorker(QObject* parent) : QObject(parent), socket_(new QTcpSocket(this)) {
    connect(socket_, &QTcpSocket::connected,    this, &NetworkWorker::onConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &NetworkWorker::onDisconnected);
    connect(socket_, &QTcpSocket::readyRead,    this, &NetworkWorker::onReadyRead);
    connect(socket_, &QTcpSocket::errorOccurred,this, &NetworkWorker::onError);
}

void NetworkWorker::connectToServer(const QString& host, quint16 port) {
    socket_->connectToHost(host, port);
}

void NetworkWorker::disconnectFromServer() {
    socket_->disconnectFromHost();
}

void NetworkWorker::sendCommand(const QString& command) {
    if (socket_->state() != QTcpSocket::ConnectedState) return;
    socket_->write((command + "\n").toUtf8());
}

void NetworkWorker::onConnected()    { emit connected(); }
void NetworkWorker::onDisconnected() { emit disconnected(); }

void NetworkWorker::onError(QAbstractSocket::SocketError) {
    emit errorOccurred(socket_->errorString());
}

void NetworkWorker::onReadyRead() {
    buffer_.append(socket_->readAll());
    int pos;
    while ((pos = buffer_.indexOf('\n')) != -1) {
        QString line = QString::fromUtf8(buffer_.left(pos)).trimmed();
        buffer_.remove(0, pos + 1);
        parseResponse(line);
    }
}

void NetworkWorker::parseResponse(const QString& response) {
    if (response == "Logging successful") {
        emit loginResult(true, response);
    } else if (response == "Login failed. Invalid details") {
        emit loginResult(false, response);
    } else if (response == "New user registered") {
        emit signupResult(true, response);
    } else if (response == "Registration failed") {
        emit signupResult(false, response);
    } else if (response == "Ship successfully placed") {
        emit shipPlaced(true, response);
    } else if (response.startsWith("Failed to place ship")) {
        emit shipPlaced(false, response);
    } else if (response.startsWith("Move result: ")) {
        emit moveResult(response.mid(13).toInt(), -1, -1);
    } else if (response == "Opponent found") {
        emit opponentFound();
    } else if (response == "Game started") {
        emit gameStarted();
    } else if (response == "Your turn") {
        emit yourTurn();
    } else if (response == "Searching opponent...") {
        emit searchingOpponent();
    } else if (response.startsWith("Opponent move: ")) {
        QStringList parts = response.mid(15).split(' ');
        if (parts.size() >= 3)
            emit opponentMove(parts[0].toInt(), parts[1].toInt(), parts[2].toInt());
    } else if (response.startsWith("Game over: ")) {
        emit gameOver(response.mid(11).toInt());
    } else if (response == "Field cleared") {
        emit fieldCleared();
    } else if (response == "Forfeited") {
        emit gameForfeited();
    } else if (response == "Opponent disconnected") {
        emit opponentLeft();
    } else if (response.startsWith("Game info: ")) {
        QStringList parts = response.mid(11).split(' ');
        if (parts.size() >= 4)
            emit gameInfo(parts[0], parts[1].toInt(), parts[2], parts[3].toInt());
    } else if (response.startsWith("Rating: ")) {
        emit ratingUpdated(response.mid(8).toInt());
    } else {
        qDebug() << "Unknown response:" << response;
    }
}
