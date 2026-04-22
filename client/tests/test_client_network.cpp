#include <QtTest/QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include "network/client_network.hpp"

class TestableClientNetwork : public ClientNetwork {
public:
    explicit TestableClientNetwork(QObject* parent = nullptr)
        : ClientNetwork(parent) {}
    void parse(const QString& r) { parseResponse(r); }
};

class TestClientNetwork : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        server_ = new QTcpServer(this);
        QVERIFY(server_->listen(QHostAddress::LocalHost, 0));
        port_ = server_->serverPort();
        connect(server_, &QTcpServer::newConnection,
                this, &TestClientNetwork::onServerConnection);
    }

    void cleanupTestCase() { server_->close(); }

    void testLoginSuccess() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::loginResult);
        net.parse("Logging successful");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), true);
    }
    void testLoginFailed() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::loginResult);
        net.parse("Login failed. Invalid details");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), false);
    }
    void testSignupSuccess() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::signupResult);
        net.parse("New user registered");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), true);
    }
    void testSignupFailed() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::signupResult);
        net.parse("Registration failed");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), false);
    }
    void testShipPlacedSuccess() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::shipPlaced);
        net.parse("Ship successfully placed");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), true);
    }
    void testShipPlacedFailed() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::shipPlaced);
        net.parse("Failed to place ship: overlap");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), false);
    }
    void testMoveResult() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::moveResult);
        net.parse("Move result: 0");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 0);
    }
    void testOpponentMove() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::opponentMove);
        net.parse("Opponent move: 3 7 1");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 3);
        QCOMPARE(spy.at(0).at(1).toInt(), 7);
        QCOMPARE(spy.at(0).at(2).toInt(), 1);
    }
    void testOpponentFound() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::opponentFound);
        net.parse("Opponent found");
        QCOMPARE(spy.count(), 1);
    }
    void testGameStarted() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::gameStarted);
        net.parse("Game started");
        QCOMPARE(spy.count(), 1);
    }
    void testYourTurn() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::yourTurn);
        net.parse("Your turn");
        QCOMPARE(spy.count(), 1);
    }
    void testGameOver() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::gameOver);
        net.parse("Game over: 42");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 42);
    }
    void testRatingUpdated() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::ratingUpdated);
        net.parse("Rating: 1050");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 1050);
    }
    void testFieldCleared() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::fieldCleared);
        net.parse("Field cleared");
        QCOMPARE(spy.count(), 1);
    }
    void testForfeited() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::gameForfeited);
        net.parse("Forfeited");
        QCOMPARE(spy.count(), 1);
    }
    void testGameInfo() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::gameInfo);
        net.parse("Game info: alice 1000 bob 990");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), QString("alice"));
        QCOMPARE(spy.at(0).at(1).toInt(), 1000);
        QCOMPARE(spy.at(0).at(2).toString(), QString("bob"));
        QCOMPARE(spy.at(0).at(3).toInt(), 990);
    }
    void testSearchingOpponent() {
        TestableClientNetwork net;
        QSignalSpy spy(&net, &ClientNetwork::searchingOpponent);
        net.parse("Searching opponent...");
        QCOMPARE(spy.count(), 1);
    }
    void testUnknownNoSignal() {
        TestableClientNetwork net;
        QSignalSpy s1(&net, &ClientNetwork::loginResult);
        QSignalSpy s2(&net, &ClientNetwork::gameStarted);
        net.parse("gibberish message xyz");
        QCOMPARE(s1.count(), 0);
        QCOMPARE(s2.count(), 0);
    }
    
    void testReceiveLoginSuccessViaTcp() {
        serverSocket_ = nullptr;
        TestableClientNetwork client;
        QSignalSpy connSpy(&client, &ClientNetwork::connected);
        client.connectToServer("127.0.0.1", port_);
        QVERIFY(connSpy.wait(2000));
        QTRY_VERIFY(serverSocket_ != nullptr);

        QSignalSpy loginSpy(&client, &ClientNetwork::loginResult);
        serverSocket_->write("Logging successful\n");
        serverSocket_->flush();
        QVERIFY(loginSpy.wait(1000));
        QCOMPARE(loginSpy.at(0).at(0).toBool(), true);
    }
    void testSendCommandDelivered() {
        serverSocket_ = nullptr;
        serverReceived_.clear();
        TestableClientNetwork client;
        QSignalSpy connSpy(&client, &ClientNetwork::connected);
        client.connectToServer("127.0.0.1", port_);
        QVERIFY(connSpy.wait(2000));

        client.sendCommand("login user pass");
        QTRY_VERIFY(!serverReceived_.isEmpty());
        QVERIFY(serverReceived_.contains("login user pass\n"));
    }

private slots:
    void onServerConnection() {
        serverSocket_ = server_->nextPendingConnection();
        connect(serverSocket_, &QTcpSocket::readyRead, this, [this]() {
            serverReceived_ += serverSocket_->readAll();
        });
    }
private:
    QTcpServer* server_ = nullptr;
    QTcpSocket* serverSocket_ = nullptr;
    QByteArray serverReceived_;
    quint16 port_ = 0;
};

QTEST_MAIN(TestClientNetwork)
#include "test_client_network.moc"
