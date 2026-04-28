#include <QtTest/QtTest>
#include "ui/field_widget.hpp"

class TestFieldWidget : public QObject {
    Q_OBJECT
private slots:
    void testInitialState() {
        FieldWidget w;
        QCOMPARE(w.getCellState(0, 0), CellState::EMPTY);
        QCOMPARE(w.getCellState(9, 9), CellState::EMPTY);
        QCOMPARE(w.getCellState(5, 5), CellState::EMPTY);
    }
    void testOutOfBoundsReturnsEmpty() {
        FieldWidget w;
        QCOMPARE(w.getCellState(-1, 0), CellState::EMPTY);
        QCOMPARE(w.getCellState(10, 0), CellState::EMPTY);
        QCOMPARE(w.getCellState(0, 10), CellState::EMPTY);
    }
    void testSetAndGetCellState() {
        FieldWidget w;
        w.setCellState(2, 3, CellState::SHIP);
        QCOMPARE(w.getCellState(2, 3), CellState::SHIP);
        w.setCellState(2, 3, CellState::HIT);
        QCOMPARE(w.getCellState(2, 3), CellState::HIT);
        w.setCellState(2, 3, CellState::MISS);
        QCOMPARE(w.getCellState(2, 3), CellState::MISS);
    }
    void testSetOutOfBoundsNoEffect() {
        FieldWidget w;
        w.setCellState(-1, 0, CellState::SHIP);
        w.setCellState(10, 0, CellState::SHIP);
        QCOMPARE(w.getCellState(0, 0), CellState::EMPTY);
    }
    void testClearField() {
        FieldWidget w;
        w.setCellState(0, 0, CellState::SHIP);
        w.setCellState(5, 5, CellState::HIT);
        w.setCellState(9, 9, CellState::MISS);
        w.clearField();
        QCOMPARE(w.getCellState(0, 0), CellState::EMPTY);
        QCOMPARE(w.getCellState(5, 5), CellState::EMPTY);
        QCOMPARE(w.getCellState(9, 9), CellState::EMPTY);
    }
    void testCellClickedSignal() {
        FieldWidget w;
        QSignalSpy spy(&w, &FieldWidget::cellClicked);

        QTest::mouseClick(&w, Qt::LeftButton, Qt::NoModifier, QPoint(15, 15));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 0);
        QCOMPARE(spy.at(0).at(1).toInt(), 0);
    }
    void testCellClickedSignalCell55() {
        FieldWidget w;
        QSignalSpy spy(&w, &FieldWidget::cellClicked);
        
        QTest::mouseClick(&w, Qt::LeftButton, Qt::NoModifier, QPoint(165, 165));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 5);
        QCOMPARE(spy.at(0).at(1).toInt(), 5);
    }
    void testSetPreviewAndClear() {
        FieldWidget w;
        QVector<QPoint> cells = { QPoint(0, 0), QPoint(1, 0), QPoint(2, 0) };
        w.setPreview(cells, QColor(255, 0, 0, 100));
        w.clearPreview();
    }
};

QTEST_MAIN(TestFieldWidget)
#include "test_field_widget.moc"
