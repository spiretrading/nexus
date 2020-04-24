#include "Spire/KeyBindings/KeyBindingsTableView.hpp"
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/IconButton.hpp"

using namespace Spire;

namespace {
  auto create_delete_button(QWidget* parent) {
    auto button_size = scale(8, 8);
    auto close_box = QRect(QPoint(0, 0), scale(8, 8));
    return new IconButton(
      imageFromSvg(":/Icons/close-purple.svg", button_size, close_box),
      imageFromSvg(":/Icons/close-red.svg", button_size, close_box), parent);
  }
}

KeyBindingsTableView::KeyBindingsTableView(QHeaderView* header,
    bool can_delete_rows, QWidget* parent)
    : ScrollArea(true, parent),
      m_header(header),
      m_can_delete_rows(can_delete_rows) {
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
  if(m_can_delete_rows) {
    auto table_layout = new QHBoxLayout();
    table_layout->setContentsMargins({});
    table_layout->setSpacing(0);
    m_delete_buttons_layout = new QVBoxLayout();
    m_delete_buttons_layout->setContentsMargins(scale_width(6),
      scale_height(9), 0, 0);
    m_delete_buttons_layout->setSpacing(scale_height(18));
    table_layout->addLayout(m_delete_buttons_layout);
    table_layout->addWidget(m_table);
    layout->addLayout(table_layout);
    on_horizontal_slider_value_changed(0);
  } else {
    layout->addWidget(m_table);
  }
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
  connect(m_table, &QTableView::clicked, this,
    &KeyBindingsTableView::on_table_clicked);
  setWidget(main_widget);
}

void KeyBindingsTableView::set_column_delegate(int column,
    KeyBindingItemDelegate* delegate) {
  m_table->setItemDelegateForColumn(column, delegate);
}

void KeyBindingsTableView::set_column_width(int column, int width) {
  m_header->resizeSection(column, width);
}

void KeyBindingsTableView::set_minimum_column_width(int column, int width) {
  m_minimum_column_widths[column] = width;
}

void KeyBindingsTableView::set_model(QAbstractTableModel* model) {
  m_header->setModel(model);
  auto old_model = m_table->model();
  auto old_selection_model = m_table->selectionModel();
  m_table->setModel(model);
  old_model->deleteLater();
  old_selection_model->deleteLater();
  if(m_can_delete_rows) {
    connect(model, &QAbstractItemModel::rowsRemoved,
      [=] (auto index, auto first, auto last) {
        update_delete_buttons(first);
      });
    connect(model, &QAbstractItemModel::dataChanged,
      [=] (auto top_left, auto top_right) {
        on_data_changed(top_left);
      });
    update_delete_buttons(0);
  }
}

void KeyBindingsTableView::set_height(int height) {
  widget()->setFixedHeight(height);
  m_table->setFixedHeight(height);
}

void KeyBindingsTableView::set_width(int width) {
  widget()->setFixedWidth(width);
  if(m_can_delete_rows) {
    width -= scale_width(26);
  }
  m_header->setFixedWidth(width);
  m_table->setFixedWidth(width);
}

bool KeyBindingsTableView::eventFilter(QObject* watched, QEvent* event) {
  if(auto button = dynamic_cast<IconButton*>(watched)) {
    if(event->type() == QEvent::MouseMove) {
      auto index = m_delete_buttons_layout->indexOf(button);
      m_table->model()->setData(m_table->model()->index(index, 0),
        QColor("#FFF1F1"), Qt::BackgroundRole);
    } else if(event->type() == QEvent::Leave) {
      auto index = m_delete_buttons_layout->indexOf(button);
      m_table->model()->setData(m_table->model()->index(index, 0),
        QVariant(), Qt::BackgroundRole);
    }
  }
  return ScrollArea::eventFilter(watched, event);
}

void KeyBindingsTableView::add_delete_button(int index) {
  auto button = create_delete_button(this);
  button->installEventFilter(this);
  m_delete_buttons_layout->insertWidget(index, button);
  button->connect_clicked_signal([=] {
    on_delete_button_clicked(index);
  });
}

void KeyBindingsTableView::update_delete_buttons(int selected_index) {
  while(auto item = m_delete_buttons_layout->takeAt(selected_index)) {
    delete item->widget();
    delete item;
  }
  for(auto i = selected_index; i < m_table->model()->rowCount() - 1; ++i) {
    add_delete_button(i);
  }
  m_delete_buttons_layout->addStretch(1);
}

void KeyBindingsTableView::on_data_changed(const QModelIndex& index) {
  if(index.row() == m_table->model()->rowCount() - 1) {
    add_delete_button(index.row());
  }
}

void KeyBindingsTableView::on_delete_button_clicked(int index) {
  m_table->model()->setData(m_table->model()->index(index, 0), QVariant(),
    Qt::BackgroundRole);
  m_table->model()->removeRow(index);
}

void KeyBindingsTableView::on_header_resize(int index, int old_size,
    int new_size) {
  if(m_table->isPersistentEditorOpen(m_table->currentIndex())) {
    m_table->closePersistentEditor(m_table->currentIndex());
    m_table->update();
  }
  if(index == 8) {
    return;
  }
  if(m_minimum_column_widths.find(index) != m_minimum_column_widths.end()) {
    auto min_width = m_minimum_column_widths[index];
    if(new_size <= min_width) {
      m_header->blockSignals(true);
      m_header->resizeSection(index, min_width);
      m_header->blockSignals(false);
    }
  }
  m_table->horizontalHeader()->resizeSection(index,
     m_header->sectionSize(index));
  auto width = [&] {
    auto width = 0;
    for(auto i = 0; i < m_table->horizontalHeader()->count(); ++i) {
      width += m_header->sectionSize(i);
    }
    return width;
  }();
  set_width(max(width, scale_width(871)));
}

void KeyBindingsTableView::on_header_move(int logical_index, int old_index,
    int new_index) {
  m_table->horizontalHeader()->moveSection(old_index, new_index);
}

void KeyBindingsTableView::on_horizontal_slider_value_changed(int new_value) {
  if(new_value != 0) {
    auto x = [&] {
      if(m_can_delete_rows) {
        // TODO: fix this magic number, clean up header layout
        return widget()->pos().x() + scale_width(21);
      }
      return widget()->pos().x();
    }();
    m_header->move(x, m_header->pos().y());
  } else {
    auto x = [&] {
      if(m_can_delete_rows) {
        // TODO: fix this magic number
        return scale_width(21);
      }
      return 0;
    }();
    m_header->move(x, m_header->pos().y());
  }
}

void KeyBindingsTableView::on_table_clicked(const QModelIndex& index) {
  auto cell_top = m_table->rowViewportPosition(index.row()) +
    m_header->height();
  auto cell_bottom = cell_top + m_table->rowHeight(index.row());
  auto region = visibleRegion();
  region.translate(0, verticalScrollBar()->value());
  if(!region.contains(QPoint(0, cell_top))) {
    ensureVisible(0, cell_top, 0, scale_height(8));
  } else if(!region.contains(QPoint(0, cell_bottom))) {
    ensureVisible(0, cell_bottom, 0, scale_height(8));
  }
  if(index.flags().testFlag(Qt::ItemIsEditable)) {
    m_table->edit(index);
  }
}
