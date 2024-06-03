#include "Spire/Ui/TableBody.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QPointer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/RowViewListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  bool test_visibility(const QWidget& container, const QRect& geometry) {
    auto widget_geometry =
      QRect(container.mapToParent(geometry.topLeft()), geometry.size());
    return std::max(0, widget_geometry.top()) <=
      std::min(container.parentWidget()->height(), widget_geometry.bottom());
  }
}

Spacing Spire::Styles::spacing(int spacing) {
  return Spacing(spacing, spacing);
}

GridColor Spire::Styles::grid_color(QColor color) {
  return GridColor(color, color);
}

QWidget* TableBody::default_item_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  auto text = std::make_shared<ToTextModel<AnyRef>>(
    std::make_shared<ListValueModel<AnyRef>>(
      std::make_shared<RowViewListModel<AnyRef>>(table, row), column),
    [] (const AnyRef& value) { return to_text(value); },
    [] (const QString&) { return none; });
  return make_label(text);
}

struct TableBody::Cover : QWidget {
  QColor m_background_color;

  Cover(QWidget* parent)
      : QWidget(parent),
        m_background_color(Qt::transparent) {
    update_style(*this, [] (auto& style) {
      style.get(CurrentRow()).set(BackgroundColor(QColor(0xE2E0FF)));
    });
  }
};

struct TableBody::RowCover : Cover {
  RowCover(TableBody& body)
      : Cover(&body) {
    make_hbox_layout(this);
    match(*this, Row());
    layout()->setSpacing(body.m_styles.m_horizontal_spacing);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    for(auto column = 0; column != body.get_column_size(); ++column) {
      auto item = new TableItem();
      body.m_hover_observers.emplace(std::piecewise_construct,
        std::forward_as_tuple(item), std::forward_as_tuple(*item));
      body.m_hover_observers.at(item).connect_state_signal(
        std::bind_front(&TableBody::on_hover, &body, std::ref(*item)));
      if(column != body.get_column_size() - 1) {
        item->setFixedWidth(
          body.m_widths->get(column) - body.get_left_spacing(column));
      } else {
        item->setSizePolicy(
          QSizePolicy::Expanding, item->sizePolicy().verticalPolicy());
      }
      layout()->addWidget(item);
      item->connect_active_signal(std::bind_front(
        &TableBody::on_item_activated, &body, std::ref(*item)));
    }
  }

  const TableItem* get_item(int index) const {
    return const_cast<RowCover*>(this)->get_item(index);
  }

  TableItem* get_item(int index) {
    if(auto item = layout()->itemAt(index)) {
      return static_cast<TableItem*>(item->widget());
    }
    return nullptr;
  }

  void mount(int index) {
    auto& body = *static_cast<TableBody*>(parentWidget());
    for(auto i = 0; i != layout()->count(); ++i) {
      get_item(i)->mount(*body.m_item_builder.mount(body.m_table, index, i));
    }
  }

  void unmount() {
    auto& body = *static_cast<TableBody*>(parentWidget());
    for(auto i = 0; i != layout()->count(); ++i) {
      body.m_item_builder.unmount(get_item(i)->unmount());
    }
  }
};

struct TableBody::ColumnCover : Cover {
  QPointer<QWidget> m_hovered;

  ColumnCover(QWidget* parent)
      : Cover(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    update_style(*this, [] (auto& style) {
      style.get(CurrentColumn()).set(BackgroundColor(QColor(0xE2E0FF)));
    });
  }
};

struct TableBody::Painter {
  static void paint_horizontal_borders(TableBody& body, QPainter& painter) {
    if(body.m_styles.m_horizontal_grid_color.alphaF() == 0) {
      return;
    }
    auto paint_border = [&] (int top, int height) {
      painter.fillRect(QRect(0, top, body.width(), height),
        body.m_styles.m_horizontal_grid_color);
    };
    if(body.m_styles.m_padding.top() != 0) {
      paint_border(0, body.m_styles.m_padding.top());
    }
    if(body.m_styles.m_vertical_spacing != 0) {
      auto bottom_index = [&] {
        if(body.m_bottom_spacer) {
          return body.layout()->count() - 1;
        }
        return body.layout()->count();
      }();
      auto top_index = 1;
      if(body.m_top_spacer) {
        paint_border(body.m_top_spacer->geometry().bottom() + 1 -
          body.m_styles.m_vertical_spacing, body.m_styles.m_vertical_spacing);
        ++top_index;
      }
      for(auto row = top_index; row < bottom_index; ++row) {
        paint_border(body.layout()->itemAt(row)->geometry().top() -
          body.m_styles.m_vertical_spacing, body.m_styles.m_vertical_spacing);
      }
      if(body.m_bottom_spacer) {
        paint_border(body.m_bottom_spacer->geometry().top(),
          body.m_styles.m_vertical_spacing);
      }
    }
    if(body.m_styles.m_padding.bottom() != 0) {
      paint_border(body.height() - body.m_styles.m_padding.bottom(),
        body.m_styles.m_padding.bottom());
    }
  }

  static void paint_vertical_borders(TableBody& body, QPainter& painter) {
    if(body.m_styles.m_vertical_grid_color.alphaF() == 0) {
      return;
    }
    auto paint_border = [&] (int left, int width) {
      painter.fillRect(QRect(left, 0, width, body.height()),
        body.m_styles.m_vertical_grid_color);
    };
    if(body.m_styles.m_padding.left() != 0) {
      paint_border(0, body.m_styles.m_padding.left());
    }
    if(body.m_styles.m_horizontal_spacing != 0 &&
        body.m_widths->get_size() > 0) {
      auto left = body.m_widths->get(0);
      for(auto column = 1; column < body.get_column_size(); ++column) {
        paint_border(left, body.m_styles.m_horizontal_spacing);
        if(column != body.m_widths->get_size()) {
          left += body.m_widths->get(column);
        }
      }
    }
    if(body.m_styles.m_padding.right() != 0) {
      paint_border(body.width() - body.m_styles.m_padding.right(),
        body.m_styles.m_padding.right());
    }
  }

  static void paint_current_item(
      TableBody& body, QPainter& painter, const optional<TableIndex>& current) {
    auto current_item = body.get_current_item();
    if(!current_item) {
      return;
    }
    auto column_cover = body.m_column_covers[current->m_column];
    if(column_cover->m_background_color.alphaF() != 0) {
      painter.fillRect(
        column_cover->geometry(), column_cover->m_background_color);
    }
    auto& row_cover = *static_cast<Cover*>(current_item->parentWidget());
    if(row_cover.m_background_color.alphaF() != 0) {
      painter.fillRect(row_cover.geometry(), row_cover.m_background_color);
    }
    auto current_position =
      current_item->parentWidget()->mapToParent(current_item->pos());
    auto& styles = current_item->get_styles();
    painter.fillRect(
      QRect(current_position, current_item->size()), styles.m_background_color);
  }

  static void paint_item_borders(
      TableBody& body, QPainter& painter, const optional<Index>& index) {
    auto item = body.find_item(index);
    if(!item) {
      return;
    }
    auto top_spacing = body.get_top_spacing(index->m_row);
    auto left_spacing = body.get_left_spacing(index->m_column);
    auto right_spacing = [&] {
      if(index->m_column == body.get_column_size() - 1) {
        return body.m_styles.m_padding.right();
      }
      return body.m_styles.m_horizontal_spacing;
    }();
    auto bottom_spacing = [&] {
      if(index->m_row == body.layout()->count() - 1) {
        return body.m_styles.m_padding.bottom();
      }
      return body.m_styles.m_vertical_spacing;
    }();
    auto get_border_size = [] (auto size) {
      if(size <= 0) {
        return 1;
      }
      return size;
    };
    auto top_border_size = get_border_size(top_spacing);
    auto left_border_size = get_border_size(left_spacing);
    auto right_border_size = get_border_size(right_spacing);
    auto bottom_border_size = get_border_size(bottom_spacing);
    auto position = item->parentWidget()->mapToParent(item->pos());
    auto& styles = item->get_styles();
    auto left = position.x() - left_spacing;
    auto top = position.y() - top_spacing;
    auto width = item->width() + left_spacing + right_spacing;
    auto height = item->height() + top_spacing + bottom_spacing;
    painter.fillRect(
      QRect(left, top, width, top_border_size), styles.m_border_top_color);
    auto right_border_x = [&] {
      auto right = position.x() + item->width();
      if(right_spacing == 0) {
        return right - 1;
      }
      return right;
    }();
    painter.fillRect(QRect(right_border_x, top, right_border_size, height),
      styles.m_border_right_color);
    auto bottom_border_y = [&] {
      auto bottom = position.y() + item->height();
      if(bottom_spacing == 0) {
        return bottom - 1;
      }
      return bottom;
    }();
    painter.fillRect(QRect(left, bottom_border_y, width, bottom_border_size),
      styles.m_border_bottom_color);
    painter.fillRect(QRect(left, top, left_border_size, height),
      styles.m_border_left_color);
  }
};

TableBody::TableBody(
    std::shared_ptr<TableModel> table, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    std::shared_ptr<ListModel<int>> widths, TableViewItemBuilder item_builder,
    QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_current_controller(std::move(current), 0, widths->get_size() + 1),
      m_selection_controller(std::move(selection), 0, widths->get_size() + 1),
      m_widths(std::move(widths)),
      m_item_builder(std::move(item_builder)),
      m_top_index(-1),
      m_visible_count(0),
      m_top_spacer(nullptr),
      m_bottom_spacer(nullptr),
      m_current_row(nullptr) {
  setFocusPolicy(Qt::StrongFocus);
  make_vbox_layout(this);
  layout()->setAlignment(Qt::AlignTop);
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableBody::on_style, this));
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_width(1))).
      set(horizontal_padding(scale_width(1))).
      set(vertical_padding(scale_height(1))).
      set(grid_color(QColor(0xE0E0E0)));
  });
  for(auto row = 0; row != m_table->get_row_size(); ++row) {
    add_row(row);
  }
  auto left = 0;
  for(auto column = 0; column != m_widths->get_size() + 1; ++column) {
    auto width = [&] {
      if(column != m_widths->get_size()) {
        return m_widths->get(column);
      }
      return this->width() - left;
    }();
    add_column_cover(column, QRect(QPoint(left, 0), QSize(width, height())));
    left += width;
  }
  if(auto current = m_current_controller.get_current()->get()) {
    match(*m_column_covers[current->m_column], CurrentColumn());
  }
  m_table_connection = m_table->connect_operation_signal(
    std::bind_front(&TableBody::on_table_operation, this));
  m_current_connection = m_current_controller.connect_update_signal(
     std::bind_front(&TableBody::on_current, this));
  m_selection_controller.connect_row_operation_signal(
    std::bind_front(&TableBody::on_row_selection, this));
  m_widths_connection = m_widths->connect_operation_signal(
    std::bind_front(&TableBody::on_widths_update, this));
}

const std::shared_ptr<TableModel>& TableBody::get_table() const {
  return m_table;
}

const std::shared_ptr<TableBody::CurrentModel>& TableBody::get_current() const {
  return m_current_controller.get_current();
}

const std::shared_ptr<TableBody::SelectionModel>&
    TableBody::get_selection() const {
  return m_selection_controller.get_selection();
}

bool TableBody::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize) {
    update_visible_region();
  }
  return QWidget::eventFilter(watched, event);
}

bool TableBody::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    auto result = QWidget::event(event);
    auto left = 0;
    for(auto column = 0; column != static_cast<int>(m_column_covers.size());
        ++column) {
      auto cover = m_column_covers[column];
      cover->move(left, 0);
      auto width = [&] {
        if(column != m_widths->get_size()) {
          return m_widths->get(column);
        }
        return this->width() - left;
      }();
      width = std::max(0, width);
      cover->setFixedSize(width, height());
      left += width;
      cover->raise();
    }
    return result;
  } else if(event->type() == QEvent::ParentChange) {
    update_parent();
  }
  return QWidget::event(event);
}

void TableBody::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Home:
      if(event->modifiers() & Qt::KeyboardModifier::ControlModifier) {
        m_current_controller.navigate_home_row();
      } else {
        m_current_controller.navigate_home_column();
      }
      break;
    case Qt::Key_End:
      if(event->modifiers() & Qt::KeyboardModifier::ControlModifier) {
        m_current_controller.navigate_end_row();
      } else {
        m_current_controller.navigate_end_column();
      }
      break;
    case Qt::Key_Up:
      m_current_controller.navigate_previous_row();
      break;
    case Qt::Key_Down:
      m_current_controller.navigate_next_row();
      break;
    case Qt::Key_Left:
      m_current_controller.navigate_previous_column();
      break;
    case Qt::Key_Right:
      m_current_controller.navigate_next_column();
      break;
    case Qt::Key_A:
      if(event->modifiers() & Qt::Modifier::CTRL && !event->isAutoRepeat()) {
        m_selection_controller.select_all();
      } else {
        event->ignore();
      }
      break;
    case Qt::Key_Control:
      m_keys.insert(Qt::Key_Control);
      m_selection_controller.set_mode(
        TableSelectionController::Mode::INCREMENTAL);
      break;
    case Qt::Key_Shift:
      m_keys.insert(Qt::Key_Shift);
      m_selection_controller.set_mode(TableSelectionController::Mode::RANGE);
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void TableBody::keyReleaseEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Control:
      m_keys.erase(Qt::Key_Control);
      if(m_selection_controller.get_mode() ==
          TableSelectionController::Mode::INCREMENTAL) {
        if(m_keys.count(Qt::Key_Shift) == 1) {
          m_selection_controller.set_mode(
            TableSelectionController::Mode::RANGE);
        } else {
          m_selection_controller.set_mode(
            TableSelectionController::Mode::SINGLE);
        }
      }
      break;
    case Qt::Key_Shift:
      m_keys.erase(Qt::Key_Shift);
      if(m_selection_controller.get_mode() ==
          TableSelectionController::Mode::RANGE) {
        if(m_keys.count(Qt::Key_Control) == 1) {
          m_selection_controller.set_mode(
            TableSelectionController::Mode::INCREMENTAL);
        } else {
          m_selection_controller.set_mode(
            TableSelectionController::Mode::SINGLE);
        }
      }
      break;
  }
}

void TableBody::moveEvent(QMoveEvent* event) {
  update_visible_region();
}

void TableBody::showEvent(QShowEvent* event) {
  update_parent();
}

void TableBody::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(m_styles.m_background_color.alphaF() != 0) {
    painter.fillRect(QRect(QPoint(0, 0), size()), m_styles.m_background_color);
  }
  auto current = m_current_controller.get_current()->get();
  for(auto i = 0; i != static_cast<int>(m_column_covers.size()); ++i) {
    auto cover = m_column_covers[i];
    if(cover->m_background_color.alphaF() != 0 &&
        (!current || current->m_column != i)) {
      painter.fillRect(cover->geometry(), cover->m_background_color);
    }
  }
  for(auto i = 0; i != layout()->count(); ++i) {
    if(auto cover = static_cast<Cover*>(layout()->itemAt(i)->widget())) {
      if(cover->m_background_color.alphaF() != 0 &&
          (!current || current->m_row != i)) {
        painter.fillRect(cover->geometry(), cover->m_background_color);
      }
    }
  }
  Painter::paint_current_item(*this, painter, current);
  Painter::paint_horizontal_borders(*this, painter);
  Painter::paint_vertical_borders(*this, painter);
  Painter::paint_item_borders(*this, painter, m_hover_index);
  Painter::paint_item_borders(*this, painter, current);
  QWidget::paintEvent(event);
}

TableBody::RowCover* TableBody::find_row(int index) {
  if(get_current()->get() && index == get_current()->get()->m_row &&
      m_current_row) {
    return m_current_row;
  } else if(!is_visible(index)) {
    return nullptr;
  } else if(m_top_spacer) {
    ++index;
  }
  return static_cast<RowCover*>(
    layout()->itemAt(index - m_top_index)->widget());
}

TableItem* TableBody::find_item(const Index& index) {
  if(auto row = find_row(index.m_row)) {
    return row->get_item(index.m_column);
  }
  return nullptr;
}

TableItem* TableBody::find_item(const optional<Index>& index) {
  if(index) {
    return find_item(*index);
  }
  return nullptr;
}

TableItem* TableBody::get_current_item() {
  if(m_current_row) {
    return m_current_row->get_item(get_current()->get()->m_column);
  } else if(auto current_index = get_current()->get()) {
    if(is_visible(current_index->m_row)) {
      m_current_row = find_row(current_index->m_row);
    } else {
      m_current_row = new RowCover(*this);
      connect_style_signal(*m_current_row, std::bind_front(
        &TableBody::on_cover_style, this, std::ref(*m_current_row)));
      m_current_row->mount(current_index->m_row);
      on_cover_style(*m_current_row);
      m_current_row->move(-1000, -1000);
      m_current_row->show();
    }
    return get_current_item();
  }
  return nullptr;
}

bool TableBody::is_visible(int index) const {
  return index >= m_top_index && index < m_top_index + m_visible_count;
}

TableBody::Index TableBody::get_index(const TableItem& item) const {
  auto row = item.parentWidget();
  if(row == m_current_row) {
    return Index(get_current()->get()->m_row,
      row->layout()->indexOf(&const_cast<TableItem&>(item)));
  }
  auto row_index = m_top_index;
  for(auto i = 0; i != layout()->count(); ++i) {
    if(auto item = layout()->itemAt(i)->widget()) {
      if(row == item) {
        break;
      }
      ++row_index;
    }
  }
  return Index(
    row_index, row->layout()->indexOf(&const_cast<TableItem&>(item)));
}

int TableBody::get_column_size() const {
  return m_widths->get_size() + 1;
}

int TableBody::get_left_spacing(int index) const {
  if(index == 0) {
    return m_styles.m_padding.left();
  }
  return m_styles.m_horizontal_spacing;
}

int TableBody::get_top_spacing(int index) const {
  if(index == 0) {
    return m_styles.m_padding.top();
  }
  return m_styles.m_vertical_spacing;
}

void TableBody::add_column_cover(int index, const QRect& geometry) {
  auto cover = new ColumnCover(this);
  cover->move(geometry.topLeft());
  cover->setFixedSize(geometry.size());
  m_column_covers.insert(m_column_covers.begin() + index, cover);
  match(*cover, Column());
  connect_style_signal(*cover,
    std::bind_front(&TableBody::on_cover_style, this, std::ref(*cover)));
  cover->show();
  on_cover_style(*cover);
}

void TableBody::add_row(int index) {
  m_current_controller.add_row(index);
  m_selection_controller.add_row(index);
}

void TableBody::remove_row(int index) {
  if(auto row = find_row(index)) {
    for(auto i = 0; i != get_column_size(); ++i) {
      auto item = row->get_item(i);
      m_hover_observers.erase(item);
    }
    auto item = layout()->takeAt(layout()->indexOf(row));
    layout()->removeItem(item);
    row->unmount();
    if(row == m_current_row) {
      m_current_row = nullptr;
    }
    delete item;
    delete row;
  }
  if(m_hover_index && m_hover_index->m_row >= index) {
    m_hover_index = none;
  }
  m_current_controller.remove_row(index);
  m_selection_controller.remove_row(index);
}

void TableBody::move_row(int source, int destination) {
/* TODO
  if(auto row = find_row(source)) {
    auto item = layout()->takeAt(layout()->indexOf(row));
    row_layout.insertItem(destination, &row);
  }
*/
  m_current_controller.move_row(source, destination);
  m_selection_controller.move_row(source, destination);
}

void TableBody::update_parent() {
  if(auto parent = parentWidget()) {
    parent->installEventFilter(this);
  }
  initialize_visible_region();
}

TableBody::RowCover* TableBody::mount_row(
    int index, int layout_index, std::vector<RowCover*>& unmounted_rows) {
  auto row = [&] {
    if(index == get_current()->get().value_or(Index(-1, -1)).m_row &&
        m_current_row) {
      return m_current_row;
    }
    if(unmounted_rows.empty()) {
      auto row = new RowCover(*this);
      connect_style_signal(*row,
        std::bind_front(&TableBody::on_cover_style, this, std::ref(*row)));
      return row;
    }
    auto row = unmounted_rows.back();
    unmounted_rows.pop_back();
    return row;
  }();
  if(row != m_current_row) {
    row->mount(index);
  }
  static_cast<QBoxLayout*>(layout())->insertWidget(layout_index, row);
  if(row != m_current_row) {
    on_cover_style(*row);
  }
  row->show();
  auto layout_event = QEvent(QEvent::LayoutRequest);
  QApplication::sendEvent(this, &layout_event);
  ++m_visible_count;
  if(!m_current_row &&
      index == get_current()->get().value_or(Index(-1, 0)).m_row) {
    m_current_row = row;
  }
  return row;
}

void TableBody::update_spacer(QSpacerItem*& spacer, int hidden_row_count) {
  auto delete_spacer = false;
  if(m_visible_count > 0 && hidden_row_count > 0) {
    auto total_height = 0;
    for(auto i = 0; i != layout()->count(); ++i) {
      if(auto row = layout()->itemAt(i)->widget()) {
        total_height += row->sizeHint().height();
      }
    }
    auto spacer_size = (hidden_row_count * total_height) / m_visible_count +
      hidden_row_count * layout()->spacing();
    if(spacer_size > 0) {
      if(spacer) {
        spacer->changeSize(0, spacer_size,
          spacer->sizePolicy().horizontalPolicy(),
          spacer->sizePolicy().verticalPolicy());
        layout()->invalidate();
      } else {
        spacer = new QSpacerItem(
          0, spacer_size, QSizePolicy::Expanding, QSizePolicy::Fixed);
        if(spacer == m_top_spacer) {
          static_cast<QBoxLayout*>(layout())->insertItem(0, spacer);
        } else {
          layout()->addItem(spacer);
        }
      }
    } else if(spacer) {
      delete_spacer = true;
    }
  } else if(spacer) {
    delete_spacer = true;
  }
  if(delete_spacer) {
    if(spacer == m_top_spacer) {
      layout()->takeAt(0);
    } else {
      layout()->takeAt(layout()->count() - 1);
    }
    delete spacer;
    spacer = nullptr;
  }
}

void TableBody::update_spacers() {
  update_spacer(m_top_spacer, m_top_index);
  update_spacer(
    m_bottom_spacer, m_table->get_row_size() - m_top_index - m_visible_count);
}

void TableBody::mount_visible_rows(std::vector<RowCover*>& unmounted_rows) {
  auto top_index = [&] {
    if(m_top_spacer) {
      return 1;
    }
    return 0;
  }();
  auto position = [&] {
    if(layout()->isEmpty()) {
      return layout()->contentsMargins().top();
    }
    return layout()->itemAt(top_index)->geometry().top() -
      layout()->spacing() - 1;
  }();
  auto top = mapFromParent(QPoint(0, 0)).y();
  while(m_top_index > 0 && position > top) {
    auto row = mount_row(m_top_index - 1, top_index, unmounted_rows);
    --m_top_index;
    position -= row->height() + layout()->spacing();
  }
  position = [&] {
    if(layout()->isEmpty()) {
      return layout()->contentsMargins().top();
    }
    auto bottom_index = [&] {
      if(m_bottom_spacer) {
        return layout()->count() - 2;
      }
      return layout()->count() - 1;
    }();
    return layout()->itemAt(bottom_index)->geometry().bottom() +
      layout()->spacing() + 1;
  }();
  auto bottom = mapFromParent(QPoint(0, parentWidget()->height())).y();
  while(m_top_index + m_visible_count < m_table->get_row_size() &&
      position < bottom) {
    auto layout_index = [&] {
      if(m_bottom_spacer) {
        return layout()->count() - 1;
      }
      return layout()->count();
    }();
    auto row =
      mount_row(m_top_index + m_visible_count, layout_index, unmounted_rows);
    position += row->height() + layout()->spacing();
  }
  update_spacers();
}

void TableBody::initialize_visible_region() {
  if(!parentWidget() || m_top_index != -1 || !isVisible() ||
      m_table->get_row_size() == 0 || m_table->get_column_size() == 0) {
    return;
  }
  m_top_index = 0;
  m_visible_count = 0;
  auto unmounted_rows = std::vector<RowCover*>();
  mount_visible_rows(unmounted_rows);
  if(get_current()->get()) {
    get_current_item();
  }
  if(m_visible_count == 0) {
    m_top_index = -1;
  }
}

std::vector<TableBody::RowCover*> TableBody::unmount_hidden_rows() {
  auto unmounted_rows = std::vector<RowCover*>();
  auto is_top = true;
  auto i = 0;
  auto r = std::vector<QLayoutItem*>();
  while(i != layout()->count()) {
    auto item = layout()->itemAt(i);
    auto row = static_cast<RowCover*>(item->widget());
    if(!row) {
      ++i;
    } else if(test_visibility(*this, row->geometry())) {
      is_top = false;
      ++i;
    } else {
      if(is_top) {
        ++m_top_index;
      }
      r.push_back(item);
      ++i;
    }
  }
  for(auto item : r) {
    auto row = static_cast<RowCover*>(item->widget());
    layout()->removeItem(item);
    delete item;
    if(row != m_current_row) {
      row->unmount();
      unmounted_rows.push_back(row);
    } else {
      row->move(-1000, -1000);
    }
    --m_visible_count;
  }
  update_spacers();
  return unmounted_rows;
}

void TableBody::update_visible_region() {
  if(m_top_index == -1) {
    initialize_visible_region();
    return;
  }
  if(!parentWidget() || !isVisible()) {
    return;
  }
  auto unmounted_rows = unmount_hidden_rows();
  if(m_visible_count != 0) {
    mount_visible_rows(unmounted_rows);
  }
  for(auto unmounted_row : unmounted_rows) {
    delete unmounted_row;
  }
}

void TableBody::on_item_activated(TableItem& item) {
  auto& row_widget = *item.parentWidget();
  auto index =
    Index(layout()->indexOf(&row_widget), row_widget.layout()->indexOf(&item));
  if(m_current_controller.get_current()->get() != index) {
    m_current_controller.get_current()->set(index);
    if(auto current = m_current_controller.get_current()->get()) {
      index = *current;
    } else {
      return;
    }
  }
  m_selection_controller.click(index);
}

void TableBody::on_current(
    const optional<Index>& previous, const optional<Index>& current) {
  if(previous) {
    if(auto previous_item = find_item(previous)) {
      unmatch(*previous_item->parentWidget(), CurrentRow());
      if(!current || current->m_column != previous->m_column) {
        unmatch(*m_column_covers[previous->m_column], CurrentColumn());
      }
      unmatch(*previous_item, Current());
      if(!is_visible(previous->m_row)) {
        m_current_row->unmount();
        delete m_current_row;
      }
      m_current_row = nullptr;
    }
  }
  if(current) {
    auto current_item = get_current_item();
    match(*current_item, Current());
    match(*current_item->parentWidget(), CurrentRow());
    if(!previous || previous->m_column != current->m_column) {
      match(*m_column_covers[current->m_column], CurrentColumn());
    }
    m_selection_controller.navigate(*current);
    current_item->setFocus();
  }
}

void TableBody::on_row_selection(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::AddOperation& operation) {
      auto& selection =
        m_selection_controller.get_selection()->get_row_selection();
      if(auto row = find_row(selection->get(operation.m_index))) {
        match(*row, Selected());
      }
    },
    [&] (const ListModel<int>::UpdateOperation& operation) {
      if(auto previous = find_row(operation.get_previous())) {
        unmatch(*previous, Selected());
      }
      if(auto row = find_row(operation.get_value())) {
        match(*row, Selected());
      }
    });
}

void TableBody::on_hover(TableItem& item, HoverObserver::State state) {
  auto index = get_index(item);
  if(state == HoverObserver::State::NONE) {
    unmatch(item, HoverItem());
    if(index == m_hover_index) {
      m_hover_index = none;
    }
    update();
  } else if(is_set(state, HoverObserver::State::MOUSE_OVER)) {
    match(item, HoverItem());
    m_hover_index = index;
    update();
  }
}

void TableBody::on_style() {
  auto& stylist = find_stylist(*this);
  m_styles = {};
  m_styles.m_background_color = Qt::transparent;
  m_styles.m_horizontal_grid_color = Qt::transparent;
  m_styles.m_vertical_grid_color = Qt::transparent;
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_background_color = color;
        });
      },
      [&] (const HorizontalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_styles.m_horizontal_spacing = std::max(0, spacing);
        });
      },
      [&] (const VerticalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_styles.m_vertical_spacing = std::max(0, spacing);
        });
      },
      [&] (const HorizontalGridColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_horizontal_grid_color = color;
        });
      },
      [&] (const VerticalGridColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_vertical_grid_color = color;
        });
      },
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_styles.m_padding.setTop(std::max(0, size));
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_styles.m_padding.setRight(std::max(0, size));
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_styles.m_padding.setBottom(std::max(0, size));
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_styles.m_padding.setLeft(std::max(0, size));
        });
      });
  }
  layout()->setSpacing(m_styles.m_vertical_spacing);
  layout()->setContentsMargins(m_styles.m_padding);
  for(auto i = 0; i != layout()->count(); ++i) {
    if(auto row = static_cast<RowCover*>(layout()->itemAt(i)->widget())) {
      row->layout()->setSpacing(m_styles.m_horizontal_spacing);
      for(auto column = 0; column != m_widths->get_size(); ++column) {
        auto& item = *row->get_item(column);
        item.setFixedWidth(m_widths->get(column) - get_left_spacing(column));
      }
    }
  }
  update_visible_region();
  update();
}

void TableBody::on_cover_style(Cover& cover) {
  auto& stylist = find_stylist(cover);
  cover.m_background_color = Qt::transparent;
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=, &cover] (auto color) {
          cover.m_background_color = color;
        });
      });
  }
  update();
}

void TableBody::on_table_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      add_row(operation.m_index);
    },
    [&] (const TableModel::PreRemoveOperation& operation) {
      remove_row(operation.m_index);
    },
    [&] (const TableModel::MoveOperation& operation) {
      move_row(operation.m_source, operation.m_destination);
    });
  update_visible_region();
}

void TableBody::on_widths_update(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      auto spacing = get_left_spacing(operation.m_index);
      for(auto i = 0; i != layout()->count(); ++i) {
        if(auto row = static_cast<RowCover*>(layout()->itemAt(i)->widget())) {
          if(auto item = row->get_item(operation.m_index)) {
            item->setFixedWidth(m_widths->get(operation.m_index) - spacing);
          }
        }
      }
    });
}
