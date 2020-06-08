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
    auto button_size = scale(16, 16);
    auto close_box = QRect(QPoint(4, 4), scale(8, 8));
    return new IconButton(
      imageFromSvg(":/Icons/close-purple.svg", button_size, close_box),
      imageFromSvg(":/Icons/close-red.svg", button_size, close_box), parent);
  }

  auto DELETE_ROW_LAYOUT_WIDTH() {
    static auto width = scale_width(26);
    return width;
  }

  auto HEADER_PADDING() {
    static auto padding = scale_width(17);
    return padding;
  }

  auto TABLE_PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }

  auto bounded_value(int value, int max) {
    if(value < 0) {
      return max;
    } else if(value > max) {
      return 0;
    }
    return value;
  }
}

KeyBindingsTableView::KeyBindingsTableView(QHeaderView* header,
    bool can_delete_rows, QWidget* parent)
    : ScrollArea(true, parent),
      m_header(header),
      m_can_delete_rows(can_delete_rows),
      m_is_default_cell_selected(true),
      m_is_editing_cell(false) {
  m_header->setParent(this);
  m_header->setStretchLastSection(false);
  auto main_widget = new QWidget(this);
  connect(m_header, &QHeaderView::sectionResized, this,
    &KeyBindingsTableView::on_header_resize);
  connect(m_header, &QHeaderView::sectionMoved, this,
    &KeyBindingsTableView::on_header_move);
  m_table = [&] {
    auto table = new CustomGridTableView(main_widget);
    auto table_padding_left = [&] {
      if(m_can_delete_rows) {
        return 0;
      }
      return TABLE_PADDING();
    }();
    table->setStyleSheet(QString(R"(
      QTableView {
        background-color: #FFFFFF;
        font-family: Roboto;
        font-size: %1px;
        gridline-color: #C8C8C8;
        outline: 0;
        padding-bottom: %2px;
      })").arg(scale_height(12)).arg(scale_height(8)));
    table->setFrameShape(QFrame::NoFrame);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->horizontalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(
      m_header->stretchLastSection());
    table->verticalHeader()->setDefaultSectionSize(scale_height(26));
    table->verticalHeader()->hide();
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    table->viewport()->setMouseTracking(true);
    table->installEventFilter(this);
    table->viewport()->installEventFilter(this);
    return table;
  }();
  if(m_can_delete_rows) {
    m_header->move(HEADER_PADDING(), 0);
    m_table->move(DELETE_ROW_LAYOUT_WIDTH(), m_header->height());
    m_delete_buttons_widget = new QWidget(main_widget);
    m_delete_buttons_widget->setFixedWidth(DELETE_ROW_LAYOUT_WIDTH());
    m_delete_buttons_widget->move(0, m_header->height());
    m_delete_buttons_layout = new QVBoxLayout(m_delete_buttons_widget);
    m_delete_buttons_layout->setContentsMargins(scale_width(5),
      scale_height(5), scale_width(7), 0);
    m_delete_buttons_layout->setSpacing(scale_height(10));
  } else {
    m_table->move(TABLE_PADDING(), m_header->height());
  }
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
    &KeyBindingsTableView::on_horizontal_slider_value_changed);
  connect(m_table, &QTableView::clicked, this,
    &KeyBindingsTableView::edit_cell);
  connect(m_table, &QTableView::activated, this,
    &KeyBindingsTableView::edit_cell);
  setWidget(main_widget);
  setFocusProxy(m_table);
  m_navigation_keys = {Qt::Key_Tab, Qt::Key_Backtab, Qt::Key_Left,
    Qt::Key_Right, Qt::Key_Up, Qt::Key_Down};
}

void KeyBindingsTableView::set_column_delegate(int column,
    KeyBindingItemDelegate* delegate) {
  delegate->setParent(m_table);
  connect(delegate, &QAbstractItemDelegate::closeEditor,
    [=] (auto editor, auto hint) {
      if(hint == QAbstractItemDelegate::EditNextItem) {
        auto next = get_next_editable_index(m_table->currentIndex());
          edit_cell(next);
      } else if(hint == QAbstractItemDelegate::EditPreviousItem) {
        auto previous = get_previous_editable_index(m_table->currentIndex());
          edit_cell(previous);
      }
    });
  m_table->setItemDelegateForColumn(column, delegate);
}

void KeyBindingsTableView::set_column_width(int column, int width) {
  m_header->resizeSection(column, width);
}

void KeyBindingsTableView::set_minimum_column_width(int column, int width) {
  m_minimum_column_widths[column] = width;
}

void KeyBindingsTableView::set_model(KeyBindingsTableModel* model) {
  m_header->setModel(model);
  auto old_model = m_table->model();
  auto old_selection_model = m_table->selectionModel();
  m_table->setModel(model);
  old_model->deleteLater();
  old_selection_model->deleteLater();
  if(m_can_delete_rows) {
    connect(model, &QAbstractItemModel::rowsRemoved,
      [=] (auto index, auto first, auto last) {
        on_row_removed(first);
      });
    connect(model, &QAbstractItemModel::rowsInserted,
      [=] (auto top_left, auto bottom_right) {
        on_row_inserted();
      });
    update_delete_buttons(0);
  }
  connect(m_table->selectionModel(),
    &QItemSelectionModel::currentColumnChanged, this,
    &KeyBindingsTableView::on_column_selection_changed);
  connect(m_table->selectionModel(),
    &QItemSelectionModel::currentChanged, this,
    &KeyBindingsTableView::on_selection_changed);
}

void KeyBindingsTableView::set_height(int height) {
  widget()->setFixedHeight(height);
  m_table->setFixedHeight(height);
  if(m_can_delete_rows) {
    m_delete_buttons_widget->setFixedHeight(height);
  }
}

void KeyBindingsTableView::set_width(int width) {
  m_header->setFixedWidth(width);
  widget()->setFixedWidth(width);
}

bool KeyBindingsTableView::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table && event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(m_navigation_keys.contains(static_cast<Qt::Key>(e->key()))) {
      auto current_index = [&] {
        if(m_is_default_cell_selected) {
          return get_first_editable_index();
        } else {
          auto index = m_table->currentIndex();
          if(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Right) {
            return get_next_editable_index(index);
          } else if(e->key() == Qt::Key_Backtab || e->key() == Qt::Key_Left) {
            return get_previous_editable_index(index);
          } else if(e->key() == Qt::Key_Up) {
            return m_table->model()->index(bounded_value(index.row() - 1,
              m_table->model()->rowCount() - 1), index.column());
          } else if(e->key() == Qt::Key_Down) {
            return m_table->model()->index(bounded_value(index.row() + 1,
              m_table->model()->rowCount() - 1), index.column());
          }
        }
        return QModelIndex();
      }();
      m_is_default_cell_selected = false;
      m_is_editing_cell = false;
      auto editor_open = m_table->isPersistentEditorOpen(
        m_table->currentIndex());
      m_table->setCurrentIndex(current_index);
      scroll_to_index(current_index);
      auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
      table_model->set_focus_highlight(current_index);
      if(!editor_open) {
        if(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
          edit_cell(current_index);
        }
      }
      update();
      return true;
    } else if(e->key() == Qt::Key_Delete) {
      m_table->model()->setData(m_table->currentIndex(), QVariant(),
        Qt::DisplayRole);
      return true;
    }
  } else if(auto button = dynamic_cast<IconButton*>(watched)) {
    if(event->type() == QEvent::MouseMove) {
      auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
      auto index = m_delete_buttons_layout->indexOf(button);
      table_model->set_row_highlight(index);
      update();
    } else if(event->type() == QEvent::Leave) {
      auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
      table_model->reset_row_highlight();
      update();
    }
  } else if(watched == m_table->viewport()) {
    if(event->type() == QEvent::MouseMove) {
      auto pos = static_cast<QMouseEvent*>(event)->pos();
      auto index = m_table->indexAt(pos);
      auto table_model = static_cast<KeyBindingsTableModel*>(
        m_table->model());
      if(index.isValid() && index.flags().testFlag(Qt::ItemIsEditable) &&
          !m_table->isPersistentEditorOpen(index)) {
        table_model->set_hover_highlight(index);
      } else {
        table_model->reset_hover_highlight();
      }
    } else if(event->type() == QEvent::Leave) {
      auto table_model = static_cast<KeyBindingsTableModel*>(
        m_table->model());
      table_model->reset_hover_highlight();
    }
  }
  return ScrollArea::eventFilter(watched, event);
}

void KeyBindingsTableView::hideEvent(QHideEvent* event) {
  m_table->closePersistentEditor(m_table->currentIndex());
}

void KeyBindingsTableView::add_delete_button(int index) {
  auto button = create_delete_button(this);
  button->installEventFilter(this);
  m_delete_buttons_layout->insertWidget(index, button);
  button->connect_clicked_signal([=] {
    on_delete_button_clicked(index);
  });
}

void KeyBindingsTableView::edit_cell(const QModelIndex& index) {
  if(index.flags().testFlag(Qt::ItemIsEditable)) {
    m_is_default_cell_selected = false;
    m_is_editing_cell = false;
    m_table->setCurrentIndex(index);
    auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
    table_model->set_focus_highlight(index);
    m_is_editing_cell = true;
    m_table->edit(index);
  }
}

QModelIndex KeyBindingsTableView::get_next_editable_index(
    const QModelIndex& index) const {
  auto next_index = index;
  do {
    next_index = get_editable_index(next_index.row(),
      m_header->visualIndex(next_index.column()) + 1);
  } while(!m_table->model()->flags(next_index).testFlag(Qt::ItemIsEditable));
  return next_index;
}

QModelIndex KeyBindingsTableView::get_previous_editable_index(
    const QModelIndex& index) const {
  auto previous_index = index;
  do {
    previous_index = get_editable_index(previous_index.row(),
      m_header->visualIndex(previous_index.column()) - 1);
  } while(!m_table->model()->flags(previous_index).testFlag(
    Qt::ItemIsEditable));
  return previous_index;
}

QModelIndex KeyBindingsTableView::get_editable_index(int row,
    int column_visual_index) const {
  if(column_visual_index > m_table->model()->columnCount() - 1) {
    ++row;
  } else if(column_visual_index < 0) {
    --row;
  }
  auto bounded_row = bounded_value(row,
    m_table->model()->rowCount() - 1);
  auto column = m_table->horizontalHeader()->logicalIndex(
    bounded_value(column_visual_index, m_table->model()->columnCount() - 1));
  return m_table->model()->index(bounded_row, column);
}

QModelIndex KeyBindingsTableView::get_first_editable_index() const {
  if(m_table->model()->rowCount() == 0) {
    return QModelIndex();
  }
  for(auto i = 0; i < m_table->model()->columnCount(); ++i) {
    if(m_table->model()->index(0, i).flags().testFlag(Qt::ItemIsEditable)) {
      return m_table->model()->index(0, i);
    }
  }
  return QModelIndex();
}

void KeyBindingsTableView::scroll_to_index(const QModelIndex& index) {
  auto cell_top = m_table->rowViewportPosition(index.row());
  auto cell_bottom = cell_top + m_table->rowHeight(index.row()) +
    m_header->height();
  auto region = visibleRegion();
  region.translate(0, verticalScrollBar()->value());
  if(!region.contains(QPoint(0, cell_top - m_header->height()))) {
    ensureVisible(0, cell_top, 0, scale_height(8));
  } else if(!region.contains(QPoint(0, cell_bottom))) {
    ensureVisible(0, cell_bottom, 0, scale_height(8));
  }
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
  //TODO: workaround for ignored repaint
  setStyleSheet(styleSheet());
}

void KeyBindingsTableView::on_column_selection_changed(
    const QModelIndex &current, const QModelIndex &previous) {
  if(m_is_editing_cell) {
    m_is_editing_cell = false;
    m_table->setCurrentIndex(previous);
    auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
    table_model->set_focus_highlight(previous);
    update();
  }
}

void KeyBindingsTableView::on_delete_button_clicked(int index) {
  auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
  table_model->reset_row_highlight();
  m_table->model()->removeRow(index);
}

void KeyBindingsTableView::on_header_resize(int index, int old_size,
    int new_size) {
  if(m_table->isPersistentEditorOpen(m_table->currentIndex())) {
    m_table->closePersistentEditor(m_table->currentIndex());
    m_table->update();
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
  m_header->setFixedWidth(width + TABLE_PADDING());
  m_table->setFixedWidth(width);
  widget()->setFixedWidth(width + TABLE_PADDING() + DELETE_ROW_LAYOUT_WIDTH());
}

void KeyBindingsTableView::on_header_move(int logical_index, int old_index,
    int new_index) {
  m_table->horizontalHeader()->moveSection(old_index, new_index);
}

void KeyBindingsTableView::on_horizontal_slider_value_changed(int new_value) {
  if(new_value != 0) {
    auto x = [&] {
      if(m_can_delete_rows) {
        return widget()->pos().x() + HEADER_PADDING();
      }
      return widget()->pos().x();
    }();
    m_header->move(x, m_header->pos().y());
  } else {
    auto x = [&] {
      if(m_can_delete_rows) {
        return HEADER_PADDING();
      }
      return 0;
    }();
    m_header->move(x, m_header->pos().y());
  }
}

void KeyBindingsTableView::on_row_inserted() {
  auto current = m_table->currentIndex();
  auto previous_index = m_table->model()->index(current.row() - 1,
    current.column());
  m_table->setCurrentIndex(previous_index);
  auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
  table_model->set_focus_highlight(previous_index);
  add_delete_button(m_table->model()->rowCount() - 2);
}

void KeyBindingsTableView::on_row_removed(int row) {
  update_delete_buttons(row);
  if(row == m_table->model()->rowCount() - 1) {
    --row;
  }
  if(row == m_table->selectionModel()->currentIndex().row()) {
    m_is_default_cell_selected = true;
    m_table->clearSelection();
    auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
    table_model->reset_focus_highlight();
    m_table->setFocus();
  }
}

void KeyBindingsTableView::on_selection_changed(const QModelIndex& current,
    const QModelIndex& previous) {
  if(!m_is_default_cell_selected) {
    auto table_model = static_cast<KeyBindingsTableModel*>(m_table->model());
    table_model->set_focus_highlight(current);
  }
}
