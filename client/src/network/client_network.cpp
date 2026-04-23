#include "network/client_network.hpp"
#include <QDebug>

ClientNetwork::ClientNetwork(QObject* parent) : QObject(parent) {
    networkWorker_ = new NetworkWorker();
    networkWorker_->moveToThread(&networkThread_);

    connect(&networkThread_, &QThread::finished, networkWorker_, &QObject::deleteLater);

    connect(this, &ClientNetwork::connectRequested,    networkWorker_, &NetworkWorker::connectToServer);
    connect(this, &ClientNetwork::disconnectRequested, networkWorker_, &NetworkWorker::disconnectFromServer);
    connect(this, &ClientNetwork::commandRequested,    networkWorker_, &NetworkWorker::sendCommand);

    connect(networkWorker_, &NetworkWorker::connected,         this, [this]{ connected_ = true; emit connected(); });
    connect(networkWorker_, &NetworkWorker::disconnected,      this, [this]{ connected_ = false; emit disconnected(); });
    connect(networkWorker_, &NetworkWorker::errorOccurred,     this, &ClientNetwork::errorOccurred);
    connect(networkWorker_, &NetworkWorker::loginResult,       this, &ClientNetwork::loginResult);
    connect(networkWorker_, &NetworkWorker::signupResult,      this, &ClientNetwork::signupResult);
    connect(networkWorker_, &NetworkWorker::shipPlaced,        this, &ClientNetwork::shipPlaced);
    connect(networkWorker_, &NetworkWorker::moveResult,        this, &ClientNetwork::moveResult);
    connect(networkWorker_, &NetworkWorker::opponentFound,     this, &ClientNetwork::opponentFound);
    connect(networkWorker_, &NetworkWorker::gameStarted,       this, &ClientNetwork::gameStarted);
    connect(networkWorker_, &NetworkWorker::yourTurn,          this, &ClientNetwork::yourTurn);
    connect(networkWorker_, &NetworkWorker::gameForfeited,     this, &ClientNetwork::gameForfeited);
    connect(networkWorker_, &NetworkWorker::gameInfo,          this, &ClientNetwork::gameInfo);
    connect(networkWorker_, &NetworkWorker::opponentMove,      this, &ClientNetwork::opponentMove);
    connect(networkWorker_, &NetworkWorker::gameOver,          this, &ClientNetwork::gameOver);
    connect(networkWorker_, &NetworkWorker::fieldCleared,      this, &ClientNetwork::fieldCleared);
    connect(networkWorker_, &NetworkWorker::searchingOpponent, this, &ClientNetwork::searchingOpponent);
    connect(networkWorker_, &NetworkWorker::ratingUpdated,     this, &ClientNetwork::ratingUpdated);
    connect(networkWorker_, &NetworkWorker::opponentLeft,      this, &ClientNetwork::opponentLeft);

    networkThread_.start();
}

ClientNetwork::~ClientNetwork() {
    networkThread_.quit();
    networkThread_.wait();
}

void ClientNetwork::connectToServer(const QString& host, quint16 port) {
    emit connectRequested(host, port);
}

void ClientNetwork::disconnectFromServer() {
    emit disconnectRequested();
}

void ClientNetwork::sendCommand(const QString& command) {
    emit commandRequested(command);
}

void ClientNetwork::parseResponse(const QString& response) {
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
