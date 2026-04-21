#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVector>

class ClientNetwork;
class FieldWidget;

class FieldSettingPage : public QWidget {
    Q_OBJECT

public:
    explicit FieldSettingPage(ClientNetwork* network, QWidget* parent = nullptr);

signals:
    void gameStarted();

private slots:
    void onReadyClicked();
    void onGameStarted();
    void onClearFieldClicked();
    void onFieldCleared();
    void onCellClicked(int x, int y);
    void onCellHover(int x, int y);

private:
    void setupUI();
    void selectShip(int type);
    void rotateShip();
    bool canPlaceShip(int row, int col, int length, bool horizontal);
    void placeShipAt(int row, int col, int length, bool horizontal);
    void highlightPreview(int row, int col);
    void updateFieldDisplay();
    void checkReady();
    void clearLocalField();

    ClientNetwork* network_;
    FieldWidget* fieldWidget_;
    QPushButton* readyButton_;
    QPushButton* clearButton_;
    QLabel* statusLabel_;
    
    QVector<QPushButton*> shipButtons_;
    QVector<QVector<int>> field_;
    QVector<int> placedShips_;
    int selectedShipType_;
    bool horizontal_;
};
