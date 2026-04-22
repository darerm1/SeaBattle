#include <QtTest/QtTest>
#include "ui/field_setting_page.hpp"
#include "network/client_network.hpp"

class MockNetwork : public ClientNetwork {
    Q_OBJECT
public:
    QString lastCommand;
    void sendCommand(const QString& cmd) override { lastCommand = cmd; }
};

class TestFieldSettingPage : public QObject {
    Q_OBJECT
private slots:
    void testInitialFieldEmpty() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        for (int y = 0; y < 10; ++y)
            for (int x = 0; x < 10; ++x)
                QCOMPARE(page.field_[y][x], 0);
    }
    void testCanPlaceShipValid() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        QVERIFY(page.canPlaceShip(0, 0, 4, true));
        QVERIFY(page.canPlaceShip(0, 0, 1, false));
        QVERIFY(page.canPlaceShip(9, 9, 1, true));
    }
    void testCanPlaceShipOutOfBounds() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        
        QVERIFY(!page.canPlaceShip(0, 7, 4, true));
        
        QVERIFY(!page.canPlaceShip(8, 0, 3, false));
    }
    void testCanPlaceShipOccupied() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placeShipAt(0, 0, 2, true);
        QVERIFY(!page.canPlaceShip(0, 0, 1, true));
        QVERIFY(!page.canPlaceShip(0, 1, 1, true));
    }
    void testCanPlaceShipAdjacentBlocked() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placeShipAt(2, 2, 1, true);
        
        QVERIFY(!page.canPlaceShip(1, 1, 1, true));
        QVERIFY(!page.canPlaceShip(2, 3, 1, true));
        QVERIFY(!page.canPlaceShip(3, 2, 1, true));
    }
    void testPlaceShipUpdatesField() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placeShipAt(0, 0, 3, true);
        QCOMPARE(page.field_[0][0], 1);
        QCOMPARE(page.field_[0][1], 1);
        QCOMPARE(page.field_[0][2], 1);
        QCOMPARE(page.field_[0][3], 0);
    }
    void testPlaceShipVertical() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placeShipAt(0, 0, 3, false);
        QCOMPARE(page.field_[0][0], 1);
        QCOMPARE(page.field_[1][0], 1);
        QCOMPARE(page.field_[2][0], 1);
        QCOMPARE(page.field_[3][0], 0);
    }
    void testReadyButtonInitiallyDisabled() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        QVERIFY(!page.readyButton_->isEnabled());
    }
    void testCheckReadyEnablesButton() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placedShips_ = {1, 2, 3, 4};
        page.checkReady();
        QVERIFY(page.readyButton_->isEnabled());
    }
    void testCheckReadyNotEnoughShips() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placedShips_ = {1, 2, 2, 4};
        page.checkReady();
        QVERIFY(!page.readyButton_->isEnabled());
    }
    void testResetPageClearsField() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placeShipAt(0, 0, 4, true);
        page.placedShips_ = {1, 0, 0, 0};
        page.resetPage();
        QCOMPARE(page.field_[0][0], 0);
        QCOMPARE(page.field_[0][3], 0);
        QVERIFY(!page.readyButton_->isEnabled());
    }
    void testResetPageReenablesShipButtons() {
        MockNetwork mock;
        FieldSettingPage page(&mock);
        page.placedShips_ = {1, 2, 3, 4};
        page.updateShipButtonsState();
        
        QVERIFY(!page.shipButtons_[0]->isEnabled());
        page.resetPage();
        
        QVERIFY(page.shipButtons_[0]->isEnabled());
        QVERIFY(page.shipButtons_[3]->isEnabled());
    }
};

QTEST_MAIN(TestFieldSettingPage)
#include "test_field_setting_page.moc"
