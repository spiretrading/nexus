#include "Spire/Ui/TableBody.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QPointer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FixedHorizontalLayout.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto SCROLL_BUFFER = 1;

  bool test_visibility(const QWidget& container, const QRect& geometry) {
    auto widget_geometry =
      QRect(container.mapToParent(geometry.topLeft()), geometry.size());
    return std::max(-SCROLL_BUFFER, widget_geometry.top()) <=
      std::min(container.parentWidget()->height() + SCROLL_BUFFER,
        widget_geometry.bottom());
  }

  int compute_average_row_height(
      QSpacerItem* top, QSpacerItem* bottom, QLayout* layout, int top_index,
      int visible_count, int rows) {
    auto total_height = 0;
    auto total_rows = 0;
    if(top) {
      total_height += top->sizeHint().height();
      total_rows += top_index;
    }
    if(layout) {
      for(auto i = 0; i != layout->count(); ++i) {
        total_height += layout->itemAt(i)->sizeHint().height();
        ++total_rows;
      }
    }
    if(bottom) {
      total_height += bottom->sizeHint().height();
      total_rows += rows - top_index - visible_count;
    }
    if(total_height == 0) {
      return 0;
    }
    return total_height / total_rows;
  }

  void set_height(QSpacerItem& spacer, QLayout& layout, int height) {
    height = std::max(0, height);
    if(spacer.sizeHint().height() == height) {
      return;
    }
    spacer.changeSize(0, height, spacer.sizePolicy().horizontalPolicy(),
      spacer.sizePolicy().verticalPolicy());
    layout.invalidate();
  }

  void adjust_height(QSpacerItem& spacer, QLayout& layout, int height) {
    set_height(spacer, layout, spacer.sizeHint().height() + height);
  }

  void move_current_index(int source, int destination, optional<int>& index) {
    if(!index) {
      return;
    }
    auto direction = source < destination ? -1 : 1;
    if(*index == source) {
      *index = destination;
    } else if(direction == 1 && *index >= destination &&
        *index < source || direction == -1 && *index > source &&
        *index <= destination) {
      *index += direction;
    }
  }

  bool focus_current(TableBody::CurrentModel& current, TableBody& body,
      Qt::FocusReason reason) {
    auto index = current.get();
    if(!index) {
      return false;
    }
    auto item = body.find_item(*index);
    if(!item) {
      return false;
    }
    item->setFocus(reason);
    return true;
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
  auto text = make_to_text_model(
    make_table_value_model<AnyRef>(table, row, column),
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
    auto layout = new FixedHorizontalLayout(this);
    match(*this, Row());
    layout->setSpacing(body.m_styles.m_horizontal_spacing);
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
      layout->addWidget(item);
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
      auto bottom_index =
        body.layout()->count() - (body.m_bottom_spacer ? 1 : 0);
      if(body.m_top_spacer) {
        paint_border(body.m_top_spacer->geometry().bottom() + 1 -
          body.m_styles.m_vertical_spacing, body.m_styles.m_vertical_spacing);
      }
      auto top_index = body.m_top_spacer ? 2 : 1;
      for(auto row = top_index; row < bottom_index; ++row) {
        paint_border(body.layout()->itemAt(row)->geometry().top() -
          body.m_styles.m_vertical_spacing, body.m_styles.m_vertical_spacing);
      }
      if(body.m_bottom_spacer) {
        paint_border(body.m_bottom_spacer->geometry().top(),
          body.m_styles.m_vertical_spacing);
      }
      auto bottom_point = [&] {
        auto index = body.layout()->count() - (body.m_bottom_spacer ? 2 : 1);
        if(index < 0) {
          return 0;
        }
        if(auto row = body.layout()->itemAt(index)->widget()) {
          return row->geometry().bottom() + 1;
        }
        return 0;
      }();
      paint_border(bottom_point, body.m_styles.m_vertical_spacing);
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
    auto bottom_point = [&] {
      auto index = body.layout()->count() - (body.m_bottom_spacer ? 2 : 1);
      if(index < 0) {
        return 0;
      }
      if(auto row = body.layout()->itemAt(index)->widget()) {
        return row->geometry().bottom() + 1;
      }
      return 0;
    }();
    auto paint_border = [&] (int left, int width) {
      painter.fillRect(QRect(left, 0, width, bottom_point),
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
    auto top_border_size = std::max(1, top_spacing);
    auto left_border_size = std::max(1, left_spacing);
    auto right_border_size = std::max(1, right_spacing);
    auto bottom_border_size = std::max(1, bottom_spacing);
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
  if(auto current = m_current_controller.get_column()) {
    match(*m_column_covers[*current], CurrentColumn());
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

TableItem* TableBody::find_item(const Index& index) {
  if(auto row = find_row(index.m_row)) {
    if(row == get_current_row() &&
        !is_visible(*m_current_controller.get_row())) {
      auto position = layout()->contentsMargins().top() +
        *m_current_controller.get_row() * estimate_row_height();
      row->move(layout()->contentsMargins().left(), position);
    }
    return row->get_item(index.m_column);
  }
  return nullptr;
}

int TableBody::estimate_scroll_line_height() const {
  return std::max(1, estimate_row_height()) + m_styles.m_vertical_spacing;
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

bool TableBody::focusNextPrevChild(bool next) {
  if(isEnabled()) {
    if(next) {
      if(navigate_next()) {
        return true;
      }
    } else if(navigate_previous()) {
      return true;
    }
  }
  auto next_focus_widget = static_cast<QWidget*>(this);
  auto next_widget = nextInFocusChain();
  while(next_widget && next_widget != this) {
    next_widget = next_widget->nextInFocusChain();
    if(!isAncestorOf(next_widget) && next_widget->isEnabled() &&
        next_widget->focusPolicy() & Qt::TabFocus) {
      next_focus_widget = next_widget;
      if(next) {
        break;
      }
    }
  }
  next_focus_widget->setFocus(Qt::TabFocusReason);
  return true;
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
  for(auto i = 0; i != static_cast<int>(m_column_covers.size()); ++i) {
    auto cover = m_column_covers[i];
    if(cover->m_background_color.alphaF() != 0 &&
        m_current_controller.get_column() != i) {
      painter.fillRect(cover->geometry(), cover->m_background_color);
    }
  }
  auto offset = m_top_spacer ? 1 : 0;
  for(auto i = 0; i != layout()->count(); ++i) {
    if(auto cover = static_cast<Cover*>(layout()->itemAt(i)->widget())) {
      auto index = m_top_index + offset + i;
      if(cover->m_background_color.alphaF() != 0 &&
          m_current_controller.get_row() != index) {
        painter.fillRect(cover->geometry(), cover->m_background_color);
      }
    }
  }
  Painter::paint_current_item(*this, painter, m_current_controller.get());
  Painter::paint_horizontal_borders(*this, painter);
  Painter::paint_vertical_borders(*this, painter);
  Painter::paint_item_borders(*this, painter, m_hover_index);
  Painter::paint_item_borders(*this, painter, m_current_controller.get());
  QWidget::paintEvent(event);
}

TableBody::RowCover* TableBody::find_row(int index) {
  if(is_visible(index)) {
    if(m_top_spacer) {
      ++index;
    }
    return static_cast<RowCover*>(
      layout()->itemAt(index - m_top_index)->widget());
  } else if(index == m_current_controller.get_row()) {
    return get_current_row();
  }
  return nullptr;
}

TableItem* TableBody::find_item(const optional<Index>& index) {
  if(index) {
    return find_item(*index);
  }
  return nullptr;
}

TableBody::RowCover* TableBody::get_current_row() {
  if(m_current_row) {
    return m_current_row;
  } else if(m_current_controller.get_row()) {
    if(is_visible(*m_current_controller.get_row())) {
      m_current_row = find_row(*m_current_controller.get_row());
    } else {
      m_current_row = new RowCover(*this);
      connect_style_signal(*m_current_row, std::bind_front(
        &TableBody::on_cover_style, this, std::ref(*m_current_row)));
      m_current_row->mount(*m_current_controller.get_row());
      on_cover_style(*m_current_row);
      m_current_row->move(-1000, -1000);
      m_current_row->show();
    }
    if(auto column = m_current_controller.get_column()) {
      match(*m_current_row->get_item(*column), Current());
    }
    match(*m_current_row, CurrentRow());
    return m_current_row;
  }
  return nullptr;
}

TableItem* TableBody::get_current_item() {
  if(auto row = get_current_row()) {
    return row->get_item(*m_current_controller.get_column());
  }
  return nullptr;
}

int TableBody::visible_count() const {
  auto count = layout()->count();
  if(m_top_spacer) {
    --count;
  }
  if(m_bottom_spacer) {
    --count;
  }
  return count;
}

bool TableBody::is_visible(int index) const {
  return index >= m_top_index && index < m_top_index + visible_count();
}

TableBody::Index TableBody::get_index(const TableItem& item) const {
  auto row = item.parentWidget();
  if(row == m_current_row) {
    return Index(*m_current_controller.get_row(),
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

int TableBody::estimate_row_height() const {
  if(m_table->get_row_size() == 0) {
    return 0;
  }
  auto layout_height = height() - layout()->contentsMargins().top() -
    layout()->contentsMargins().bottom();
  return (layout_height + layout()->spacing()) / m_table->get_row_size();
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
  if(is_visible(index)) {
    auto layout_index = index - m_top_index + (m_top_spacer ? 1 : 0);
    auto current_index = m_current_controller.get_row();
    if(current_index && *current_index >= index) {
      ++*current_index;
    }
    mount_row(index, layout_index, current_index);
  } else {
    auto row_height = compute_average_row_height(m_top_spacer, m_bottom_spacer,
      layout(), m_top_index, visible_count(), m_table->get_row_size() - 1);
    if(index < m_top_index) {
      if(m_top_spacer) {
        adjust_height(*m_top_spacer, *layout(),  row_height);
      } else {
        m_top_spacer = new QSpacerItem(
          0, row_height, QSizePolicy::Expanding, QSizePolicy::Fixed);
        static_cast<QBoxLayout*>(layout())->insertItem(0, m_top_spacer);
      }
      ++m_top_index;
    } else {
      if(m_bottom_spacer) {
        adjust_height(*m_bottom_spacer, *layout(),  row_height);
      } else {
        m_bottom_spacer = new QSpacerItem(
          0, row_height, QSizePolicy::Expanding, QSizePolicy::Fixed);
        layout()->addItem(m_bottom_spacer);
      }
    }
  }
  m_current_controller.add_row(index);
  m_selection_controller.add_row(index);
  update_visible_region();
}

void TableBody::pre_remove_row(int index) {
  if(is_visible(index)) {
    remove(*find_row(index));
  } else {
    auto row_height = -1;
    if(index == m_current_controller.get_row() && m_current_row) {
      row_height = m_current_row->sizeHint().height();
      destroy(m_current_row);
      m_current_row = nullptr;
    }
    if(index < m_top_index) {
      if(m_top_spacer) {
        if(row_height == -1) {
          row_height = compute_average_row_height(m_top_spacer, nullptr,
            nullptr, m_top_index, visible_count(), m_table->get_row_size() - 1);
        }
        adjust_height(*m_top_spacer, *layout(), -row_height);
      }
      --m_top_index;
    } else {
      if(m_bottom_spacer) {
        if(row_height == -1) {
          row_height = compute_average_row_height(nullptr, m_bottom_spacer,
            nullptr, m_top_index, visible_count(), m_table->get_row_size() - 1);
        }
        adjust_height(*m_bottom_spacer, *layout(), -row_height);
      }
    }
  }
  if(m_hover_index && m_hover_index->m_row >= index) {
    m_hover_index = none;
  }
}

void TableBody::remove_row(int index) {
  m_current_controller.remove_row(index);
  m_selection_controller.remove_row(index);
  update_visible_region();
}

void TableBody::move_row(int source, int destination) {
  if(is_visible(source)) {
    if(is_visible(destination)) {
      if(auto row = find_row(source)) {
        auto index = destination - m_top_index + (m_top_spacer ? 1 : 0);
        auto item = layout()->takeAt(layout()->indexOf(row));
        static_cast<QBoxLayout*>(layout())->insertItem(index, item);
        auto layout_event = QEvent(QEvent::LayoutRequest);
        QApplication::sendEvent(this, &layout_event);
      }
    } else if(auto row = find_row(source)) {
      auto row_height = row->sizeHint().height();
      if(row == m_current_row) {
        auto item = layout()->takeAt(layout()->indexOf(row));
        delete item;
      } else {
        remove(*row);
      }
      if(auto spacer =
          destination < m_top_index ? m_top_spacer : m_bottom_spacer) {
        adjust_height(*spacer, *layout(), row_height);
      }
      if(destination < m_top_index) {
        ++m_top_index;
      }
    }
  } else if(source < m_top_index) {
    if(is_visible(destination)) {
      auto layout_index =
        destination - m_top_index + (m_top_spacer ? 1 : 0) + 1;
      auto current_index = m_current_controller.get_row();
      move_current_index(source, destination, current_index);
      auto row = mount_row(destination, layout_index, current_index);
      if(m_top_spacer) {
        adjust_height(*m_top_spacer, *layout(), -row->sizeHint().height());
      }
      --m_top_index;
    } else if(destination >= m_top_index + visible_count()) {
      if(m_top_spacer) {
        auto top_row_height = m_top_spacer->sizeHint().height() / m_top_index;
        adjust_height(*m_top_spacer, *layout(), -top_row_height);
        if(m_bottom_spacer) {
          adjust_height(*m_bottom_spacer, *layout(), top_row_height);
        }
      }
      --m_top_index;
    }
  } else {
    if(is_visible(destination)) {
      auto layout_index = destination - m_top_index + (m_top_spacer ? 1 : 0);
      auto current_index = m_current_controller.get_row();
      move_current_index(source, destination, current_index);
      auto row = mount_row(destination, layout_index, current_index);
      if(m_bottom_spacer) {
        adjust_height(*m_bottom_spacer, *layout(), -row->sizeHint().height());
      }
    } else if(destination < m_top_index) {
      if(m_bottom_spacer) {
        auto hidden_rows =
          m_table->get_row_size() - m_top_index - visible_count();
        if(hidden_rows > 0) {
          auto bottom_row_height =
            m_bottom_spacer->sizeHint().height() / hidden_rows;
          adjust_height(*m_bottom_spacer, *layout(), -bottom_row_height);
          if(m_top_spacer) {
            adjust_height(*m_top_spacer, *layout(), bottom_row_height);
          }
        }
      }
      ++m_top_index;
    }
  }
  m_current_controller.move_row(source, destination);
  m_selection_controller.move_row(source, destination);
  update_visible_region();
}

void TableBody::update_parent() {
  if(auto parent = parentWidget()) {
    parent->installEventFilter(this);
  }
  initialize_visible_region();
}

TableBody::RowCover* TableBody::mount_row(int index, int layout_index,
    optional<int> current_index, std::vector<RowCover*>& unmounted_rows) {
  auto row = [&] {
    if(!current_index) {
      current_index = m_current_controller.get_row();
    }
    if(index == current_index) {
      return get_current_row();
    }
    if(unmounted_rows.empty()) {
      auto row = new RowCover(*this);
      connect_style_signal(*row,
        std::bind_front(&TableBody::on_cover_style, this, std::ref(*row)));
      return row;
    }
    auto row = unmounted_rows.back();
    unmounted_rows.pop_back();
    row->setParent(this);
    return row;
  }();
  if(row != m_current_row) {
    row->mount(index);
  } else {
    assert(layout()->indexOf(m_current_row) == -1);
  }
  static_cast<QBoxLayout*>(layout())->insertWidget(layout_index, row);
  if(row != m_current_row) {
    on_cover_style(*row);
  }
  row->show();
  auto layout_event = QEvent(QEvent::LayoutRequest);
  QApplication::sendEvent(this, &layout_event);
  return row;
}

TableBody::RowCover* TableBody::mount_row(
    int index, int layout_index, optional<int> current_index) {
  auto unmounted_rows = std::vector<RowCover*>();
  return mount_row(index, layout_index, current_index, unmounted_rows);
}

void TableBody::destroy(RowCover* row) {
  for(auto i = 0; i != get_column_size(); ++i) {
    auto item = row->get_item(i);
    m_hover_observers.erase(item);
  }
  row->deleteLater();
}

void TableBody::remove(RowCover& row) {
  auto item = layout()->takeAt(layout()->indexOf(&row));
  layout()->removeItem(item);
  if(&row == m_current_row) {
    m_current_row = nullptr;
  }
  row.unmount();
  delete item;
  destroy(&row);
}

void TableBody::update_spacer(QSpacerItem*& spacer, int hidden_row_count) {
  auto delete_spacer = false;
  if(visible_count() > 0 && hidden_row_count > 0) {
    auto total_height = 0;
    for(auto i = 0; i != layout()->count(); ++i) {
      if(auto row = layout()->itemAt(i)->widget()) {
        total_height += row->sizeHint().height();
      }
    }
    auto spacer_size = (hidden_row_count * total_height) / visible_count() +
      hidden_row_count * layout()->spacing();
    if(spacer_size > 0) {
      if(spacer) {
        set_height(*spacer, *layout(), spacer_size);
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
    m_bottom_spacer, m_table->get_row_size() - m_top_index - visible_count());
}

void TableBody::mount_visible_rows(std::vector<RowCover*>& unmounted_rows) {
  auto top_index = m_top_spacer ? 1 : 0;
  auto position = [&] {
    if(layout()->isEmpty()) {
      return layout()->contentsMargins().top();
    }
    return layout()->itemAt(top_index)->geometry().top() -
      layout()->spacing() - 1;
  }();
  auto top = mapFromParent(QPoint(0, 0)).y() - SCROLL_BUFFER;
  while(m_top_index > 0 && position > top) {
    auto row = mount_row(m_top_index - 1, top_index, none, unmounted_rows);
    --m_top_index;
    position -= row->height() + layout()->spacing();
  }
  position = [&] {
    if(layout()->isEmpty()) {
      return layout()->contentsMargins().top();
    }
    auto bottom_index = layout()->count() - (m_bottom_spacer ? 2 : 1);
    return layout()->itemAt(bottom_index)->geometry().bottom() +
      layout()->spacing() + 1;
  }();
  auto bottom =
    mapFromParent(QPoint(0, parentWidget()->height())).y() + SCROLL_BUFFER;
  while(m_top_index + visible_count() < m_table->get_row_size() &&
      position < bottom) {
    auto layout_index = layout()->count() - (m_bottom_spacer ? 1 : 0);
    auto row = mount_row(
      m_top_index + visible_count(), layout_index, none, unmounted_rows);
    position += row->height() + layout()->spacing();
  }
  update_spacers();
}

std::vector<TableBody::RowCover*> TableBody::unmount_hidden_rows() {
  auto unmounted_rows = std::vector<RowCover*>();
  auto removed_items = std::vector<QLayoutItem*>();
  auto is_top = true;
  for(auto i = 0; i != layout()->count(); ++i) {
    auto& item = *layout()->itemAt(i);
    if(auto row = static_cast<RowCover*>(item.widget())) {
      if(test_visibility(*this, row->geometry())) {
        is_top = false;
      } else {
        if(is_top) {
          ++m_top_index;
        }
        removed_items.push_back(&item);
      }
    }
  }
  for(auto item : removed_items) {
    auto row = static_cast<RowCover*>(item->widget());
    layout()->removeItem(item);
    delete item;
    if(row != m_current_row) {
      row->unmount();
      unmounted_rows.push_back(row);
    } else {
      row->move(-1000, -1000);
    }
  }
  update_spacers();
  return unmounted_rows;
}

void TableBody::initialize_visible_region() {
  if(!parentWidget() || m_top_index != -1 || !isVisible() ||
      m_table->get_row_size() == 0 || m_table->get_column_size() == 0) {
    return;
  }
  m_top_index = 0;
  auto unmounted_rows = std::vector<RowCover*>();
  mount_visible_rows(unmounted_rows);
  if(m_current_controller.get_row()) {
    get_current_item();
  }
  if(visible_count() == 0) {
    m_top_index = -1;
  }
}

void TableBody::reset_visible_region(
    int total_height, std::vector<RowCover*>& unmounted_rows) {
  if(m_table->get_row_size() == 0) {
    return;
  }
  auto row_height =
    (total_height + layout()->spacing()) / m_table->get_row_size();
  if(row_height == 0) {
    return;
  }
  m_top_index = mapFromParent(QPoint(0, 0)).y() / row_height;
  auto top_position = row_height * m_top_index;
  if(top_position != 0) {
    if(!m_top_spacer) {
      m_top_spacer = new QSpacerItem(
        0, top_position, QSizePolicy::Expanding, QSizePolicy::Fixed);
      static_cast<QBoxLayout*>(layout())->insertItem(0, m_top_spacer);
    } else {
      set_height(*m_top_spacer, *layout(), top_position);
    }
  }
  auto layout_index = m_top_spacer ? 1 : 0;
  mount_row(m_top_index, layout_index, none, unmounted_rows);
  mount_visible_rows(unmounted_rows);
}

void TableBody::update_visible_region() {
  if(m_top_index == -1) {
    initialize_visible_region();
    return;
  }
  if(!parentWidget() || !isVisible()) {
    return;
  }
  auto layout_event = QEvent(QEvent::LayoutRequest);
  QApplication::sendEvent(this, &layout_event);
  auto total_height = height() -
    layout()->contentsMargins().top() - layout()->contentsMargins().bottom();
  auto unmounted_rows = unmount_hidden_rows();
  if(visible_count() != 0) {
    mount_visible_rows(unmounted_rows);
  } else {
    reset_visible_region(total_height, unmounted_rows);
  }
  for(auto unmounted_row : unmounted_rows) {
    destroy(unmounted_row);
  }
}

bool TableBody::navigate_next() {
  if(auto& current = get_current()->get()) {
    auto column = current->m_column + 1;
    if(column >= get_table()->get_column_size() - 1) {
      auto row = current->m_row + 1;
      if(row >= get_table()->get_row_size()) {
        return false;
      } else {
        get_current()->set(Index(row, 0));
      }
    } else {
      get_current()->set(Index(current->m_row, column));
    }
  } else if(get_table()->get_row_size() > 0) {
    get_current()->set(Index(0, 0));
  } else {
    return false;
  }
  return focus_current(*get_current(), *this, Qt::TabFocusReason);
}

bool TableBody::navigate_previous() {
  if(auto& current = get_current()->get()) {
    auto column = current->m_column - 1;
    if(column < 0) {
      auto row = current->m_row - 1;
      if(row < 0) {
        return false;
      } else {
        get_current()->set(Index(row, get_table()->get_column_size() - 2));
      }
    } else {
      get_current()->set(Index(current->m_row, column));
    }
  } else if(get_table()->get_row_size() > 0) {
    get_current()->set(Index(
      get_table()->get_row_size() - 1, get_table()->get_column_size() - 2));
  } else {
    return false;
  }
  return focus_current(*get_current(), *this, Qt::BacktabFocusReason);
}

void TableBody::on_item_activated(TableItem& item) {
  auto& row_widget = *item.parentWidget();
  auto index = get_index(item);
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
  auto previous_had_focus = false;
  if(previous) {
    if(auto previous_item = find_item(previous)) {
      previous_had_focus =
        previous_item->isAncestorOf(QApplication::focusWidget());
      unmatch(*previous_item->parentWidget(), CurrentRow());
      unmatch(*previous_item, Current());
    }
    if(!current || current->m_column != previous->m_column) {
      unmatch(*m_column_covers[previous->m_column], CurrentColumn());
    }
    if(m_current_row && layout()->indexOf(m_current_row) == -1) {
      m_current_row->unmount();
      destroy(m_current_row);
    }
    m_current_row = nullptr;
  }
  if(current) {
    auto current_item = get_current_item();
    match(*current_item, Current());
    match(*current_item->parentWidget(), CurrentRow());
    if(!previous || previous->m_column != current->m_column) {
      match(*m_column_covers[current->m_column], CurrentColumn());
    }
    m_selection_controller.navigate(*current);
    if(previous_had_focus || QApplication::focusWidget() == this) {
      current_item->setFocus(Qt::FocusReason::OtherFocusReason);
    }
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
      pre_remove_row(operation.m_index);
    },
    [&] (const TableModel::RemoveOperation& operation) {
      remove_row(operation.m_index);
    },
    [&] (const TableModel::MoveOperation& operation) {
      move_row(operation.m_source, operation.m_destination);
    });
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
