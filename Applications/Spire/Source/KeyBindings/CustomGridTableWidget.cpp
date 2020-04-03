#include "Spire/KeyBindings/CustomGridTableWidget.hpp"
#include <QHeaderView>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

CustomGridTableWidget::CustomGridTableWidget(int row_count, int column_count,
    QWidget* parent)
    : QTableWidget(row_count, column_count, parent) {
  setStyleSheet(QString(R"(
    QTableWidget {
      background-color: #FFFFFF;
      font-family: Roboto;
      font-size: %1px;
      gridline-color: #C8C8C8;
      outline: 0;
      padding-bottom: %4px;
      padding-left: %2px;
      padding-right: %2px;
      padding-top: %3px;
    }

    QTableWidget::item {
      border: none;
      padding-left: %2px;
    }

    QTableWidget::item:selected {
      color: black;
    })").arg(scale_height(12)).arg(scale_width(8)).arg(scale_height(30))
        .arg(scale_height(8)));
  setFrameShape(QFrame::NoFrame);
  setVerticalScrollMode(QTableWidget::ScrollPerPixel);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setColumnWidth(0, scale_width(238));
  horizontalHeader()->hide();
  horizontalHeader()->setStretchLastSection(true);
  verticalHeader()->hide();
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader()->setDefaultSectionSize(scale_height(26));
  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
    &CustomGridTableWidget::on_selection_changed);
}

void CustomGridTableWidget::paintEvent(QPaintEvent* event) {
  QTableWidget::paintEvent(event);
  auto painter = QPainter(viewport());
  painter.setPen(QColor("#C8C8C8"));
  if(rowViewportPosition(0) == 0) {
    painter.drawLine(translate(0, 0), translate(852, 0));
  }
  painter.drawLine(translate(0, 0), translate(0, 338));
  if(m_selected_index.isValid() && state() == QTableWidget::EditingState) {
    painter.setPen(QColor("#4B23A0"));
    auto [pos_y, row_height] = [&] {
      auto y = rowViewportPosition(m_selected_index.row());
      auto height = rowHeight(m_selected_index.row());
      if(m_selected_index.row() > 0) {
        return std::make_pair(y - scale_height(1), height);
      }
      return std::make_pair(y, height - scale_height(1));
    }();
    painter.drawRect(
      columnViewportPosition(m_selected_index.column()) - scale_width(1),
      pos_y, columnWidth(m_selected_index.column()), row_height);
  }
}

void CustomGridTableWidget::on_selection_changed(
    const QItemSelection &selected, const QItemSelection &deselected) {
  if(selected.empty()) {
    m_selected_index = QModelIndex();
    return;
  }
  auto index = selected.indexes().first();
  if(index.column() == 1) {
    m_selected_index = index;
  }
}
