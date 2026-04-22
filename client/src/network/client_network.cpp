#include "network/client_network.hpp"
#include <QHostAddress>
#include <QDebug>

ClientNetwork::ClientNetwork(QObject* parent) : QObject(parent), socket_(std::make_unique<QTcpSocket>(this)) {
    connect(socket_.get(), &QTcpSocket::connected, this, &ClientNetwork::onConnected);
    connect(socket_.get(), &QTcpSocket::disconnected, this, &ClientNetwork::onDisconnected);
    connect(socket_.get(), &QTcpSocket::readyRead, this, &ClientNetwork::onReadyRead);
    connect(socket_.get(), &QTcpSocket::errorOccurred, this, &ClientNetwork::onError);
}

ClientNetwork::~ClientNetwork() {
    if (socket_->state() == QTcpSocket::ConnectedState) {
        socket_->disconnectFromHost();
    }
}

void ClientNetwork::connectToServer(const QString& host, quint16 port) {
    socket_->connectToHost(host, port);
}

void ClientNetwork::disconnectFromServer() {
    socket_->disconnectFromHost();
}

void ClientNetwork::sendCommand(const QString& command) {
    if (!isConnected()) {
        emit errorOccurred("Not connected to server");
        return;
    }
    
    QString cmdWithNewline = command + "\n";
    socket_->write(cmdWithNewline.toUtf8());
}

void ClientNetwork::onConnected() {
    emit connected();
}

void ClientNetwork::onDisconnected() {
    emit disconnected();
}

void ClientNetwork::onReadyRead() {
    buffer_.append(socket_->readAll());
    
    int newlinePos;
    while ((newlinePos = buffer_.indexOf('\n')) != -1) {
        QByteArray line = buffer_.left(newlinePos);
        buffer_.remove(0, newlinePos + 1);
        
        QString response = QString::fromUtf8(line).trimmed();
        parseResponse(response);
    }
}

void ClientNetwork::parseResponse(const QString& response) {
    if (response == "Logging successful") {
        emit loginResult(true, response);
    }
    else if (response == "Login failed. Invalid details") {
        emit loginResult(false, response);
    }
    else if (response == "New user registered") {
        emit signupResult(true, response);
    }
    else if (response == "Registration failed") {
        emit signupResult(false, response);
    }
    else if (response == "Ship successfully placed") {
        emit shipPlaced(true, response);
    }
    else if (response.startsWith("Failed to place ship")) {
        emit shipPlaced(false, response);
    }
    else if (response.startsWith("Move result: ")) {
        int result = response.mid(13).toInt();
        emit moveResult(result, -1, -1);
    }
    else if (response == "Opponent found") {
        emit opponentFound();
    }
    else if (response == "Game started") {
        emit gameStarted();
    }
    else if (response == "Your turn") {
        emit yourTurn();
    }
    else if (response == "Searching opponent...") {
        emit searchingOpponent();
    }
    else if (response == "Waiting for opponent...") {
    }
    else if (response.startsWith("Opponent move: ")) {
        QStringList parts = response.mid(15).split(' ');
        if (parts.size() >= 3) {
            emit opponentMove(parts[0].toInt(), parts[1].toInt(), parts[2].toInt());
        }
    }
    else if (response.startsWith("Game over: ")) {
        emit gameOver(response.mid(11).toInt());
    }
    else if (response == "Field cleared") {
        emit fieldCleared();
    }
    else if (response == "Forfeited") {
        emit gameForfeited();
    }
    else if (response == "Opponent disconnected") {
        emit opponentLeft();
    }
    else if (response.startsWith("Game info: ")) {
        QStringList parts = response.mid(11).split(' ');
        if (parts.size() >= 4) {
            emit gameInfo(parts[0], parts[1].toInt(), parts[2], parts[3].toInt());
        }
    }
    else if (response.startsWith("Rating: ")) {
        emit ratingUpdated(response.mid(8).toInt());
    }
    else {
        qDebug() << "Unknown response:" << response;
    }
}

void ClientNetwork::onError(QAbstractSocket::SocketError error) {
    emit errorOccurred(socket_->errorString());
}
