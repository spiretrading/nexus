#include "Spire/KeyBindings/CustomGridTableView.hpp"
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  const auto PLACEHOLDER_ROW_COUNT = 10;
}

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
  if(model()->rowCount() == 0 || horizontalHeader()->count() == 0) {
    return;
  }
  auto painter = QPainter(viewport());
  painter.setPen(QColor("#C8C8C8"));
  if(rowViewportPosition(0) == 0) {
    painter.drawLine(0, 0, width(), 0);
  }
  painter.drawLine(0, 0, 0, rowViewportPosition(model()->rowCount() - 1) +
    rowHeight(model()->rowCount() - 1) - scale_height(1));
  if(model()->rowCount() < PLACEHOLDER_ROW_COUNT) {
    painter.setPen(QColor("#EBEBEB"));
    auto row_height = rowHeight(0);
    auto row_y = rowViewportPosition(model()->rowCount() - 1) + row_height;
    for(auto i = model()->rowCount(); i < PLACEHOLDER_ROW_COUNT; ++i) {
      painter.drawLine(0, row_y, 0, row_y + row_height);
      for(auto column = 0; column < horizontalHeader()->count(); ++column) {
        auto column_x = horizontalHeader()->sectionViewportPosition(column) - 1;
        painter.drawLine(column_x, row_y, column_x, row_y + row_height);
      }
      auto last_column_end_x = horizontalHeader()->sectionViewportPosition(
        horizontalHeader()->count() - 1) +
        horizontalHeader()->sectionSize(horizontalHeader()->count() - 1) - 1;
      painter.drawLine(last_column_end_x, row_y, last_column_end_x,
        row_y + row_height);
      painter.drawLine(0, row_y + row_height, width() - 1, row_y + row_height);
      row_y += row_height;
    }
  }
  if(selectionModel()->hasSelection()) {
    draw_border(selectionModel()->currentIndex(), &painter);
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
