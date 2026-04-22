#include "ui/field_widget.hpp"
#include <QPainter>
#include <QMouseEvent>

FieldWidget::FieldWidget(QWidget* parent) : QWidget(parent), cells_(GRID_SIZE, QVector<CellState>(GRID_SIZE, CellState::EMPTY))
                                           ,  blockedCells_(GRID_SIZE, QVector<bool>(GRID_SIZE, false)) {
    setFixedSize(GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE);
    setMouseTracking(true);
}

void FieldWidget::setCellState(int x, int y, CellState state) {
    if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
        cells_[y][x] = state;
        update();
    }
}

CellState FieldWidget::getCellState(int x, int y) const {
    if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
        return cells_[y][x];
    return CellState::EMPTY;
}

void FieldWidget::clearField() {
    for (int y = 0; y < GRID_SIZE; ++y)
        for (int x = 0; x < GRID_SIZE; ++x)
            cells_[y][x] = CellState::EMPTY;
    update();
}

void FieldWidget::setPlacementMode(bool enabled) {
    placementMode_ = enabled;
}

void FieldWidget::setPreview(const QVector<QPoint>& cells, const QColor& color) {
    previewCells_ = cells;
    previewColor_ = color;
    update();
}

void FieldWidget::clearPreview() {
    previewCells_.clear();
    update();
}

void FieldWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            QRect rect = getCellRect(x, y);
            QColor color;
            switch (cells_[y][x]) {
                case CellState::SHIP: color = Qt::darkGray; break;
                case CellState::HIT:  color = Qt::red; break;
                case CellState::MISS: color = Qt::lightGray; break;
                default:              color = Qt::white; break;
            }
            painter.fillRect(rect, color);
        }
    }
    
    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i <= GRID_SIZE; ++i) {
        int pos = i * CELL_SIZE;
        painter.drawLine(0, pos, width(), pos);
        painter.drawLine(pos, 0, pos, height());
    }
    
    if (!previewCells_.isEmpty()) {
        painter.setBrush(QBrush(previewColor_));
        painter.setPen(Qt::NoPen);
        for (const QPoint& p : previewCells_) {
            QRect rect = getCellRect(p.x(), p.y());
            painter.fillRect(rect, previewColor_);
        }
    }
}

void FieldWidget::mousePressEvent(QMouseEvent* event) {
    QPoint gridPos = mapToGrid(event->pos());
    if (gridPos.x() >= 0 && gridPos.x() < GRID_SIZE &&
        gridPos.y() >= 0 && gridPos.y() < GRID_SIZE) {
        if (blockedCells_[gridPos.y()][gridPos.x()]) {
            return;
        }
        emit cellClicked(gridPos.x(), gridPos.y());
    }
}

void FieldWidget::mouseMoveEvent(QMouseEvent* event) {
    QPoint gridPos = mapToGrid(event->pos());
    if (gridPos.x() >= 0 && gridPos.x() < GRID_SIZE &&
        gridPos.y() >= 0 && gridPos.y() < GRID_SIZE) {
        emit mouseHover(gridPos.x(), gridPos.y());
    }
}

void FieldWidget::leaveEvent(QEvent* event) {
    emit mouseHover(-1, -1);
    clearPreview();
}

QPoint FieldWidget::mapToGrid(const QPoint& pos) const {
    return QPoint(pos.x() / CELL_SIZE, pos.y() / CELL_SIZE);
}

QRect FieldWidget::getCellRect(int x, int y) const {
    return QRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
}

void FieldWidget::markSunkShipCells(const QVector<QPoint>& shipCells) {
    for (const QPoint& p : shipCells) {
        setCellState(p.x(), p.y(), CellState::HIT);
    }
    
    for (const QPoint& p : shipCells) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = p.x() + dx;
                int ny = p.y() + dy;
                if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10) {
                    if (getCellState(nx, ny) == CellState::EMPTY) {
                        setCellState(nx, ny, CellState::MISS);
                        setCellBlocked(nx, ny, true);
                    }
                }
            }
        }
    }
    update();
}

bool FieldWidget::isCellBlocked(int x, int y) const {
    return blockedCells_[y][x];
}

void FieldWidget::setCellBlocked(int x, int y, bool blocked) {
    blockedCells_[y][x] = blocked;
}
