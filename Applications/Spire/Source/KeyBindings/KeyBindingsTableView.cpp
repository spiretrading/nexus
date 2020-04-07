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
    : ScrollArea(true, parent),
      m_header(header) {
  m_header->setParent(this);
  auto main_widget = new QWidget(this);
  auto layout = new QVBoxLayout(main_widget);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  connect(m_header, &QHeaderView::sectionResized, this,
    &KeyBindingsTableView::on_header_resize);
  connect(m_header, &QHeaderView::sectionMoved, this,
    &KeyBindingsTableView::on_header_move);
  auto header_padding = new QWidget(this);
  header_padding->setFixedHeight(m_header->height());
  layout->addWidget(header_padding);
  m_table = new CustomGridTableView(this);
  layout->addWidget(m_table);
  m_table->setStyleSheet(QString(R"(
    QTableView {
      background-color: #FFFFFF;
      font-family: Roboto;
      font-size: %1px;
      gridline-color: #C8C8C8;
      outline: 0;
      padding-bottom: %3px;
      padding-left: %2px;
      padding-right: %2px;
    }

    QTableView::item {
      border: none;
      padding-left: %2px;
    }

    QTableView::item:selected {
      color: black;
    })").arg(scale_height(12)).arg(scale_width(8)).arg(scale_height(8)));
  m_table->setFrameShape(QFrame::NoFrame);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
    &KeyBindingsTableView::on_horizontal_slider_value_changed);
  m_table->horizontalHeader()->hide();
  m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  m_table->horizontalHeader()->setStretchLastSection(
    m_header->stretchLastSection());
  m_table->verticalHeader()->setDefaultSectionSize(scale_height(26));
  m_table->verticalHeader()->hide();
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
  connect(m_table, &QTableView::clicked, [=] (auto index) {
    if(index.flags().testFlag(Qt::ItemIsEditable)) {
      m_table->edit(index);
    }
  });
  setWidget(main_widget);
}

void KeyBindingsTableView::set_column_delegate(int column,
    QStyledItemDelegate* delegate) {
  m_table->setItemDelegateForColumn(column, delegate);
}

void KeyBindingsTableView::set_column_width(int column, int width) {
  m_header->resizeSection(column, width);
}

void KeyBindingsTableView::set_model(QAbstractTableModel* model) {
  m_header->setModel(model);
  auto old_model = m_table->model();
  auto old_selection_model = m_table->selectionModel();
  m_table->setModel(model);
  old_model->deleteLater();
  old_selection_model->deleteLater();
}

void KeyBindingsTableView::set_height(int height) {
  widget()->setFixedHeight(height);
  m_table->setFixedHeight(height);
}

void KeyBindingsTableView::set_width(int width) {
  widget()->setFixedWidth(width);
  m_header->setFixedWidth(width);
  m_table->setFixedWidth(width);
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
