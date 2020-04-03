#include "Spire/KeyBindings/KeyBindingsTableView.hpp"
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

KeyBindingsTableView::KeyBindingsTableView(QHeaderView* header,
    QWidget* parent)
    : ScrollArea(parent),
      m_header(header) {
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
    &KeyBindingsTableView::on_horizontal_slider_value_changed);
  connect(header, &QHeaderView::sectionResized, this,
    &KeyBindingsTableView::on_header_resize);
  connect(header, &QHeaderView::sectionMoved, this,
    &KeyBindingsTableView::on_header_move);
  auto main_widget = new QWidget(this);
  //main_widget->setMinimumWidth(MINIMUM_TABLE_WIDTH);
  auto layout = new QVBoxLayout(main_widget);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout = new QVBoxLayout(main_widget);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  //m_header_padding = new QWidget(this);
  //m_header_padding->setFixedHeight(m_header->height());
  //m_layout->addWidget(m_header_padding);
  m_table = new QTableView(this);
 // m_table->setMinimumWidth(MINIMUM_TABLE_WIDTH);
  m_table->setStyleSheet(QString(R"(
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
  m_table->setFrameShape(QFrame::NoFrame);
  m_table->setVerticalScrollMode(QTableView::ScrollPerPixel);
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setEditTriggers(QAbstractItemView::EditTrigger::SelectedClicked);
  connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged,
    this, &KeyBindingsTableView::on_selection_changed);
  //m_table->installEventFilter(this);
  layout->addWidget(m_table);
  setWidget(main_widget);
}

void KeyBindingsTableView::set_model(QAbstractTableModel* model) {
  m_header->setModel(model);
  m_table->setModel(model);
}

void KeyBindingsTableView::set_column_delegate(int column,
    QStyledItemDelegate* delegate) {
  m_table->setItemDelegateForColumn(column, delegate);
}

void KeyBindingsTableView::paintEvent(QPaintEvent* event) {
  ScrollArea::paintEvent(event);
  auto painter = QPainter(viewport());
  painter.setPen(QColor("#C8C8C8"));
  if(m_table->rowViewportPosition(0) == 0) {
    painter.drawLine(translate(0, 0), translate(m_table->width(), 0));
  }
  painter.drawLine(translate(0, 0), translate(0, m_table->height()));
  if(m_selected_index.isValid()) {
    painter.setPen(QColor("#4B23A0"));
    auto [pos_y, row_height] = [&] {
      auto y = m_table->rowViewportPosition(m_selected_index.row());
      auto height = m_table->rowHeight(m_selected_index.row());
      if(m_selected_index.row() > 0) {
        return std::make_pair(y - scale_height(1), height);
      }
      return std::make_pair(y, height - scale_height(1));
    }();
    painter.drawRect(
      m_table->columnViewportPosition(m_selected_index.column()) -
      scale_width(1), pos_y, m_table->columnWidth(m_selected_index.column()),
      row_height);
  }
}

void KeyBindingsTableView::on_header_resize(int index, int old_size,
    int new_size) {
  m_table->horizontalHeader()->resizeSection(index,
    m_header->sectionSize(index));
}

void KeyBindingsTableView::on_header_move(int logical_index, int old_index,
    int new_index) {
  m_table->horizontalHeader()->moveSection(old_index, new_index);
}

void KeyBindingsTableView::on_horizontal_slider_value_changed(int new_value) {
  if(new_value != 0) {
    m_header->move(widget()->pos().x(), m_header->pos().y());
  } else {
    m_header->move(0, m_header->pos().y());
  }
}

void KeyBindingsTableView::on_selection_changed(
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
