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
      border: 1px solid #C8C8C8;
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
      border: 1px solid red;
    })").arg(scale_height(12)).arg(scale_width(8)).arg(scale_height(30))
        .arg(scale_height(8)));
  setFrameShape(QFrame::NoFrame);
  setSelectionMode(QAbstractItemView::NoSelection);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setColumnWidth(0, scale_width(238));
  horizontalHeader()->hide();
  horizontalHeader()->setStretchLastSection(true);
  verticalHeader()->hide();
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader()->setDefaultSectionSize(scale_height(26));
}

void CustomGridTableWidget::paintEvent(QPaintEvent* event) {
  QTableWidget::paintEvent(event);
  auto painter = QPainter(viewport());
  painter.setPen(QColor("#C8C8C8"));
  painter.drawLine(translate(0, 0), translate(852, 0));
  painter.drawLine(translate(0, 0), translate(0, 338));
}
