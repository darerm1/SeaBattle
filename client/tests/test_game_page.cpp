#include <QtTest/QtTest>
#include "ui/game_page.hpp"
#include "network/client_network.hpp"

class MockNetwork : public ClientNetwork {
    Q_OBJECT
public:
    QString lastCommand;
    void sendCommand(const QString& cmd) override { lastCommand = cmd; }
};

class TestGamePage : public QObject {
    Q_OBJECT
private slots:
    void testInitFieldSetsShips() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        field[0][0] = 1;
        field[5][3] = 1;
        page.initField(field);
        QCOMPARE(page.ownFieldWidget_->getCellState(0, 0), CellState::SHIP);
        QCOMPARE(page.ownFieldWidget_->getCellState(3, 5), CellState::SHIP);
        QCOMPARE(page.ownFieldWidget_->getCellState(1, 0), CellState::EMPTY);
    }
    void testInitFieldResetsState() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        QVERIFY(!page.myTurn_);
        QVERIFY(!page.gameOver_);
    }
    void testYourTurnSetsTurn() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        QVERIFY(!page.myTurn_);
        emit mock.yourTurn();
        QVERIFY(page.myTurn_);
        QCOMPARE(page.turnLabel_->text(), QString("Ваш ход"));
    }
    void testMoveResultHit() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.moveResult(0, 3, 4);
        QCOMPARE(page.enemyFieldWidget_->getCellState(3, 4), CellState::HIT);
        QVERIFY(page.myTurn_);
    }
    void testMoveResultMiss() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.moveResult(1, 2, 3);
        QCOMPARE(page.enemyFieldWidget_->getCellState(2, 3), CellState::MISS);
        QVERIFY(!page.myTurn_);
    }
    void testMoveResultSunk() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.moveResult(2, 1, 1);
        QCOMPARE(page.enemyFieldWidget_->getCellState(1, 1), CellState::HIT);
        QVERIFY(page.myTurn_);
    }
    void testMoveResultGameOver() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.moveResult(3, 0, 0);
        QVERIFY(page.gameOver_);
        QCOMPARE(page.turnLabel_->text(), QString("Вы победили!"));
    }
    void testOpponentMoveHit() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.opponentMove(5, 6, 0);
        QCOMPARE(page.ownFieldWidget_->getCellState(5, 6), CellState::HIT);
        QVERIFY(!page.myTurn_);
    }
    void testOpponentMoveMiss() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.opponentMove(7, 8, 1);
        QCOMPARE(page.ownFieldWidget_->getCellState(7, 8), CellState::MISS);
        QVERIFY(page.myTurn_);
    }
    void testOpponentMoveSunk() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.opponentMove(4, 4, 2);
        QCOMPARE(page.ownFieldWidget_->getCellState(4, 4), CellState::HIT);
        QVERIFY(!page.myTurn_);
    }
    void testOpponentMoveGameOver() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.opponentMove(0, 0, 3);
        QVERIFY(page.gameOver_);
        QCOMPARE(page.turnLabel_->text(), QString("Вы проиграли!"));
    }
    void testGameOverFromForfeit() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.gameOver(1);
        QVERIFY(page.gameOver_);
        QCOMPARE(page.turnLabel_->text(), QString("Соперник вышел из игры, вы победили"));
    }
    void testGameOverAfterWin() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.moveResult(3, 0, 0);
        emit mock.gameOver(1);
        QCOMPARE(page.turnLabel_->text(), QString("Вы победили!"));
    }
    void testGameOverAfterLoss() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        emit mock.opponentMove(0, 0, 3);
        emit mock.gameOver(1);
        QCOMPARE(page.turnLabel_->text(), QString("Вы проиграли!"));
    }
    void testExitButtonWhenGameOverEmitsSignal() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);
        page.gameOver_ = true;
        QSignalSpy spy(&page, &GamePage::exitRequested);
        QTest::mouseClick(page.exitButton_, Qt::LeftButton);
        QCOMPARE(spy.count(), 1);
    }
    void testExitButtonWhenGameActivesSendsForfeit() {
        MockNetwork mock;
        GamePage page(&mock);
        QVector<QVector<int>> field(10, QVector<int>(10, 0));
        page.initField(field);

        mock.lastCommand.clear();
        QSignalSpy spy(&page, &GamePage::exitRequested);
        QTest::mouseClick(page.exitButton_, Qt::LeftButton);
        QCOMPARE(mock.lastCommand, QString("forfeit"));
        QCOMPARE(spy.count(), 0);
    }
};

QTEST_MAIN(TestGamePage)
#include "test_game_page.moc"
