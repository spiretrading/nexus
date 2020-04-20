#include "Spire/KeyBindings/CustomGridTableView.hpp"
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

CustomGridTableView::CustomGridTableView(QWidget* parent)
    : QTableView(parent) {
  setMouseTracking(true);
}

void CustomGridTableView::leaveEvent(QEvent* event) {
  m_last_mouse_pos.reset();
}

void CustomGridTableView::mouseMoveEvent(QMouseEvent* event) {
  m_last_mouse_pos = event->pos();
}

void CustomGridTableView::paintEvent(QPaintEvent* event) {
  QTableView::paintEvent(event);
  if(model()->rowCount() == 0) {
    return;
  }
  auto painter = QPainter(viewport());
  painter.setPen(QColor("#C8C8C8"));
  if(rowViewportPosition(0) == 0) {
    painter.drawLine(0, 0, width(), 0);
  }
  painter.drawLine(0, 0, 0, rowViewportPosition(model()->rowCount() - 1) +
    rowHeight(model()->rowCount() - 1) - scale_height(1));
  if(selectionModel()->hasSelection() &&
      state() == QAbstractItemView::EditingState) {
    auto index = selectionModel()->selection().indexes().first();
    draw_border(index, &painter);
  }
  if(m_last_mouse_pos) {
    auto index = indexAt(*m_last_mouse_pos);
    if(index.isValid() && index.flags().testFlag(Qt::ItemIsEditable)) {
      draw_border(index, &painter);
    }
  }
}

void CustomGridTableView::draw_border(const QModelIndex& index,
    QPainter* painter) {
  if(index.flags().testFlag(Qt::ItemIsEditable)) {
    painter->setPen(QColor("#4B23A0"));
    auto [pos_y, row_height] = [&] {
      auto y = rowViewportPosition(index.row());
      auto height = rowHeight(index.row());
      if(index.row() > 0) {
        return std::tuple(y - scale_height(1), height);
      }
      return std::tuple(y, height - scale_height(1));
    }();
    auto [pos_x, column_width] = [&] {
      auto x = columnViewportPosition(index.column());
      auto width = columnWidth(index.column());
      if(index.column() > 0) {
        return std::tuple(x - scale_width(1), width);
      }
      return std::tuple(x, width - scale_width(1));
    }();
    painter->drawRect(pos_x, pos_y, column_width, row_height);
  }
}
