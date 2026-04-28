#pragma once

#include <QWidget>
#include <QVector>
#include <QPoint>

enum class CellState {
    EMPTY,
    SHIP,
    HIT,
    MISS
};

class FieldWidget : public QWidget {
    Q_OBJECT

public:
    explicit FieldWidget(QWidget* parent = nullptr);
    
    void setCellState(int x, int y, CellState state);
    CellState getCellState(int x, int y) const;
    void clearField();
    void setPlacementMode(bool enabled);
    
    void setPreview(const QVector<QPoint>& cells, const QColor& color);
    void clearPreview();
    void markSunkShipCells(const QVector<QPoint>& shipCells);
    bool isCellBlocked(int x, int y) const;
    void setCellBlocked(int x, int y, bool blocked);

signals:
    void cellClicked(int x, int y);
    void mouseHover(int x, int y);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QPoint mapToGrid(const QPoint& pos) const;
    QRect getCellRect(int x, int y) const;
    
    static constexpr int GRID_SIZE = 10;
    static constexpr int CELL_SIZE = 30;
    
    QVector<QVector<CellState>> cells_;
    QVector<QVector<bool>> blockedCells_;
    bool placementMode_ = false;
    
    QVector<QPoint> previewCells_;
    QColor previewColor_;
};
