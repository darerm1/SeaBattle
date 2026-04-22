#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVector>

class ClientNetwork;
class FieldWidget;

struct ShipPlacement {
    int length = 0;
    int x = 0;
    int y = 0;
    bool horizontal = false;
};

class FieldSettingPage : public QWidget {
    Q_OBJECT

public:
    explicit FieldSettingPage(ClientNetwork* network, QWidget* parent = nullptr);

    const QVector<QVector<int>>& getField() const { return field_; }

    void resetPage();

signals:
    void gameStarted();
    void exitRequested();

private slots:
    void onReadyClicked();
    void onGameStarted();
    void onClearFieldClicked();
    void onFieldCleared();
    void onCellClicked(int x, int y);
    void onCellHover(int x, int y);
    void onShipPlaced(bool success, const QString& message);
    void onOpponentLeft();

private:
    friend class TestFieldSettingPage;

    void setupUI();
    void selectShip(int type);
    void rotateShip();
    bool canPlaceShip(int row, int col, int length, bool horizontal);
    void placeShipAt(int row, int col, int length, bool horizontal);
    void highlightPreview(int row, int col);
    void updateFieldDisplay();
    void checkReady();
    void clearLocalField();
    void updateShipButtonsState();

    ClientNetwork* network_;
    FieldWidget* fieldWidget_;
    QPushButton* readyButton_;
    QPushButton* clearButton_;
    QPushButton* exitButton_;
    QLabel* statusLabel_;
    
    QVector<QPushButton*> shipButtons_;
    QVector<QVector<int>> field_;
    QVector<int> placedShips_;
    int selectedShipType_;
    bool horizontal_;
    ShipPlacement lastPlacedShip_;
};
