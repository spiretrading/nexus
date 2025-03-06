#include "Spire/Ui/TableBody.hpp"
#include <ranges>
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
    [] (AnyRef value) { return to_text(value); },
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

struct TableBody::ColumnCover : Cover {
  ColumnCover(QWidget* parent)
      : Cover(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    update_style(*this, [] (auto& style) {
      style.get(CurrentColumn()).set(BackgroundColor(QColor(0xE2E0FF)));
    });
  }
};

struct TableBody::RowCover : Cover {
  RowCover(TableBody& body)
      : Cover(&body) {
    auto layout = new FixedHorizontalLayout(this);
    match(*this, Row());
    layout->setSpacing(body.m_styles.m_horizontal_spacing);
    layout->setContentsMargins(body.m_styles.m_horizontal_spacing, 0,
      body.m_styles.m_horizontal_spacing, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    for(auto column = 0; column != body.get_column_size(); ++column) {
      auto item = new TableItem();
      body.m_hover_observers.emplace(std::piecewise_construct,
        std::forward_as_tuple(item), std::forward_as_tuple(*item));
      body.m_hover_observers.at(item).connect_state_signal(
        std::bind_front(&TableBody::on_hover, &body, std::ref(*item)));
      if(column != body.get_column_size() - 1) {
        if(body.m_column_covers[column]->isVisible()) {
          item->setFixedWidth(
            body.m_widths->get(column) - body.get_left_spacing(column));
        } else {
          item->setFixedWidth(0);
        }
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
      if(auto item = get_item(i)) {
        body.m_item_builder.unmount(item->unmount());
      }
    }
  }
};

struct TableBody::Painter {
  static void paint_horizontal_borders(TableBody& body, QPainter& painter) {
    if(body.m_styles.m_horizontal_grid_color.alphaF() == 0) {
      return;
    }
    auto width = body.width() - body.m_styles.m_padding.left() -
      body.m_styles.m_padding.right();
    auto paint_border = [&] (int top) {
      painter.fillRect(QRect(body.m_styles.m_padding.left(), top, width,
        body.m_styles.m_vertical_spacing),
        body.m_styles.m_horizontal_grid_color);
    };
    if(body.m_styles.m_vertical_spacing != 0) {
/*
      for(auto i = 0; i != body.get_layout().count(); ++i) {
        auto& row = body.get_layout().get_row(i);
        paint_border(row.y() - body.m_styles.m_vertical_spacing);
      }
      if(body.get_layout().count() > 0) {
        auto& row = body.get_layout().get_row(body.get_layout().count() - 1);
        paint_border(row.y() + row.height());
      }
*/
    }
  }

  static void paint_vertical_borders(TableBody& body, QPainter& painter) {
    if(body.m_styles.m_vertical_grid_color.alphaF() == 0) {
      return;
    }
/*
    auto bottom_point = [&] {
      if(auto row = body.get_layout().itemAt(body.get_layout().count() - 1)) {
        return row->widget()->y() + row->widget()->height() -
          body.m_styles.m_padding.top();
      }
      return 0;
    }();
    auto paint_border = [&] (int left) {
      painter.fillRect(QRect(left, body.m_styles.m_padding.top(),
        body.m_styles.m_horizontal_spacing, bottom_point),
        body.m_styles.m_vertical_grid_color);
    };
    if(body.m_styles.m_horizontal_spacing != 0) {
      auto margins = body.get_layout().contentsMargins();
      auto left = margins.left();
      auto width = 0;
      for(auto column_cover : body.m_column_covers) {
        if(column_cover->isVisible()) {
          width += column_cover->width();
          if(margins.left() < width) {
            paint_border(left);
            left = width;
          }
        }
      }
      paint_border(body.width() - body.m_styles.m_padding.right() -
        body.m_styles.m_horizontal_spacing);
    }
*/
  }

  static void paint_current_item(
      TableBody& body, QPainter& painter, const optional<TableIndex>& current) {
    auto current_item = body.get_current_item();
    if(!current_item || !current_item->isVisible()) {
      return;
    }
    auto column_cover = body.m_column_covers[current->m_column];
    if(column_cover->m_background_color.alphaF() != 0) {
/*
      auto body_geometry = body.rect().adjusted(body.m_styles.m_padding.left(),
        body.m_styles.m_padding.top(), -body.m_styles.m_padding.right(),
        -body.m_styles.m_padding.bottom());
      auto bottom_point = [&] {
        if(auto row = body.get_layout().itemAt(body.get_layout().count() - 1)) {
          return row->widget()->y() + row->widget()->height();
        }
        return 0;
      }();
      auto column_cover_geometry = column_cover->geometry().adjusted(
        0, 0, body.m_styles.m_horizontal_spacing,
        bottom_point - column_cover->height());
      auto column_geometry = body_geometry.intersected(column_cover_geometry);
      painter.fillRect(column_geometry, column_cover->m_background_color);
*/
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
    if(!item || item->width() == 0) {
      return;
    }
    auto top_border_size = std::max(1, body.m_styles.m_vertical_spacing);
    auto left_border_size = std::max(1, body.m_styles.m_horizontal_spacing);
    auto right_border_size = std::max(1, body.m_styles.m_horizontal_spacing);
    auto bottom_border_size = std::max(1, body.m_styles.m_vertical_spacing);
    auto position = item->parentWidget()->mapToParent(item->pos());
    auto& styles = item->get_styles();
    auto left = [&] {
      if(body.m_styles.m_horizontal_spacing == 0) {
        return position.x();
      }
      return position.x() - left_border_size;
    }();
    auto top = [&] {
      if(body.m_styles.m_vertical_spacing == 0) {
        return position.y();
      }
      return position.y() - top_border_size;
    }();
    auto width = item->width() + 2 * body.m_styles.m_horizontal_spacing;
    auto height = item->height() + 2 * body.m_styles.m_vertical_spacing;
    painter.fillRect(
      QRect(left, top, width, top_border_size), styles.m_border_top_color);
    auto right_border_x = [&] {
      auto right = position.x() + item->width();
      if(body.m_styles.m_horizontal_spacing == 0) {
        return right - 1;
      }
      return right;
    }();
    painter.fillRect(QRect(right_border_x, top, right_border_size, height),
      styles.m_border_right_color);
    auto bottom_border_y = [&] {
      auto bottom = position.y() + item->height();
      if(body.m_styles.m_vertical_spacing == 0) {
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
      m_current_row(nullptr),
      m_is_transaction(false) {
  setFocusPolicy(Qt::StrongFocus);
  for(auto i = 0; i != m_table->get_row_size(); ++i) {
    add_row(i);
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
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableBody::on_style, this));
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_width(1))).
      set(grid_color(QColor(0xE0E0E0)));
  });
  update_contents_size();
  update_visible_region();
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
    auto current_row = m_current_controller.get_row();
/*
    if(row == get_current_row() && current_row &&
        !get_layout().is_visible(*current_row)) {
      auto position = m_styles.m_padding.top() +
        *m_current_controller.get_row() * estimate_row_height();
      row->move(m_styles.m_padding.left(), position);
    }
*/
    return row->get_item(index.m_column);
  }
  return nullptr;
}

int TableBody::estimate_scroll_line_height() const {
  return std::max(1, estimate_row_height()) + m_styles.m_vertical_spacing;
}

void TableBody::show_column(int column) {
  if(column < 0 || column >= static_cast<int>(m_column_covers.size())) {
    return;
  }
  m_column_covers[column]->show();
  update_column_covers();
  update_column_widths();
}

void TableBody::hide_column(int column) {
  if(column < 0 || column >= static_cast<int>(m_column_covers.size())) {
    return;
  }
  m_column_covers[column]->hide();
  update_column_covers();
  update_column_widths();
}

QSize TableBody::sizeHint() const {
  return m_size_hint;
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
    update_column_covers();
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
/*
  for(auto i = 0; i != get_layout().count(); ++i) {
    auto& cover = get_layout().get_row(i);
    auto index = get_layout().get_top_index() + i;
    if(cover.m_background_color.alphaF() != 0 &&
        m_current_controller.get_row() != index) {
      painter.fillRect(cover.geometry(), cover.m_background_color);
    }
  }
*/
  Painter::paint_current_item(*this, painter, m_current_controller.get());
  Painter::paint_horizontal_borders(*this, painter);
  Painter::paint_vertical_borders(*this, painter);
  Painter::paint_item_borders(*this, painter, m_hover_index);
  Painter::paint_item_borders(*this, painter, m_current_controller.get());
  QWidget::paintEvent(event);
}

void TableBody::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  update_visible_region();
  update_contents_size();
}

void TableBody::showEvent(QShowEvent* event) {
  update_parent();
  update_column_covers();
  update_column_widths();
}

TableBody::RowCover* TableBody::find_row(int index) {
  auto i = m_visible_rows.find(index);
  if(i == m_visible_rows.end()) {
    return nullptr;
  }
  return i->second;
}

TableItem* TableBody::find_item(const optional<Index>& index) {
  if(index) {
    return find_item(*index);
  }
  return nullptr;
}

TableBody::RowCover* TableBody::get_current_row() {
/*
  if(m_current_row) {
    return m_current_row;
  } else if(m_current_controller.get_row()) {
    if(get_layout().is_visible(*m_current_controller.get_row())) {
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
*/
  return nullptr;
}

TableItem* TableBody::get_current_item() {
  if(auto row = get_current_row()) {
    if(auto column = m_current_controller.get_column()) {
      return row->get_item(*column);
    }
  }
  return nullptr;
}

TableBody::Index TableBody::get_index(const TableItem& item) const {
  auto row = item.parentWidget();
  if(row == m_current_row) {
    return Index(*m_current_controller.get_row(),
      row->layout()->indexOf(&const_cast<TableItem&>(item)));
  }
  auto row_index = -1;
  for(auto& visible_row : m_visible_rows) {
    if(row == visible_row.second) {
      row_index = visible_row.first;
      break;
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
  auto layout_height =
    height() - m_styles.m_padding.top() - m_styles.m_padding.bottom();
  return (layout_height + m_styles.m_vertical_spacing) /
    m_table->get_row_size();
}

int TableBody::get_left_spacing(int index) const {
  if(m_column_covers[index]->x() == 0) {
    return m_styles.m_padding.left() + m_styles.m_horizontal_spacing;
  }
  return m_styles.m_horizontal_spacing;
}

int TableBody::get_top_spacing(int index) const {
  if(index == 0) {
    return m_styles.m_padding.top() + m_styles.m_vertical_spacing;
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
  auto new_metrics = std::unordered_map<int, RowMetrics>();
  for(auto i = m_row_metrics.begin(); i != m_row_metrics.end(); ++i) {
    if(i->first >= index) {
      new_metrics[index + 1] = i->second;
    } else {
      new_metrics[index] = i->second;
    }
  }
  m_row_metrics = new_metrics;
  m_current_controller.add_row(index);
  m_selection_controller.add_row(index);
}

void TableBody::pre_remove_row(int index) {
  if(m_hover_index && m_hover_index->m_row >= index) {
    m_hover_index = none;
  }
}

void TableBody::remove_row(int index) {
  m_current_controller.remove_row(index);
  m_selection_controller.remove_row(index);
}

void TableBody::move_row(int source, int destination) {
  m_current_controller.move_row(source, destination);
  m_selection_controller.move_row(source, destination);
}

void TableBody::update_parent() {
  if(auto parent = parentWidget()) {
    parent->installEventFilter(this);
  }
  update_visible_region();
  update_contents_size();
}

void TableBody::destroy(RowCover* row) {
//  row->hide();
  row->unmount();
  unmatch(*row, CurrentRow());
  unmatch(*row, Selected());
  m_row_pool.push_back(row);
}

int TableBody::get_row_height(int row) const {
  auto i = m_row_metrics.find(row);
  if(i != m_row_metrics.end()) {
    return i->second.m_height;
  }
  return 40;
}

void TableBody::set_row_height(int row, int height) {
  m_row_metrics[row] = RowMetrics(height, true);
}

int TableBody::estimate_total_height() const {
  auto total_height = m_styles.m_padding.top() + m_styles.m_padding.bottom();
  for(auto i = 0; i < m_table->get_row_size(); ++i) {
    total_height += get_row_height(i) + m_styles.m_vertical_spacing;
  }
  return total_height;
}

void TableBody::update_contents_size() {
  auto row_size = m_table->get_row_size();
  auto content_height = estimate_total_height();
  auto content_width = 0;
  m_size_hint = QSize(content_width, estimate_total_height());
  updateGeometry();
}

void TableBody::mount(int index) {
  if(m_visible_rows.contains(index)) {
    return;
  }
  auto row = [&] {
    if(m_row_pool.empty()) {
      auto row = new RowCover(*this);
      connect_style_signal(*row,
        std::bind_front(&TableBody::on_cover_style, this, std::ref(*row)));
      return row;
    }
    auto row = m_row_pool.front();
    m_row_pool.pop_front();
    return row;
  }();
  row->mount(index);
  on_cover_style(*row);
  m_visible_rows[index] = row;
}

void TableBody::unmount(int index) {
  auto i = m_visible_rows.find(index);
  if(i == m_visible_rows.end()) {
    return;
  }
  auto row = i->second;
  m_visible_rows.erase(i);
  set_row_height(index, row->height());
  destroy(row);
}

std::pair<int, int> TableBody::compute_visible_range() const {
  auto parent_size = [&] {
    if(auto parent = parentWidget()) {
      return parent->size();
    }
    return size();
  }();
  auto offset = -pos().y();
  auto row_size = m_table->get_row_size();
  if(row_size == 0) {
    return std::pair(0, 0);
  }
  auto first_visible_row = 0;
  auto total_height = m_styles.m_padding.top();
  while(first_visible_row < row_size) {
    auto row_height = get_row_height(first_visible_row);
    if(total_height + row_height > offset) {
      break;
    }
    total_height += row_height + m_styles.m_vertical_spacing;
    ++first_visible_row;
  }
  first_visible_row = qMin(first_visible_row, row_size - 1);
  auto last_visible_row = first_visible_row;
  while(last_visible_row < row_size - 1) {
    auto row_height = get_row_height(last_visible_row);
    total_height += row_height + m_styles.m_vertical_spacing;
    if(total_height > offset + parent_size.height()) {
      break;
    }
    ++last_visible_row;
  }
  return std::pair(first_visible_row, last_visible_row);
}

int TableBody::compute_row_position(int row) const {
  auto position = m_styles.m_padding.top();
  for(auto i = 0; i < row; ++i) {
    position += get_row_height(i) + m_styles.m_vertical_spacing;
  }
  return position;
}

QRect TableBody::compute_row_geometry(int row) const {
  auto offset = -pos().y();
  auto x = m_styles.m_padding.left();
  auto y = compute_row_position(row);
  return QRect(x, y,
    width() - m_styles.m_padding.left() - m_styles.m_padding.right(),
    get_row_height(row));
}

void TableBody::layout_rows() {
  auto row_max_heights = std::unordered_map<int, int>();
  for(auto [position, row] : m_visible_rows) {
    auto height = row->sizeHint().height();
    if(!row_max_heights.contains(position) ||
        row_max_heights[position] < height) {
      row_max_heights[position] = height;
    }
  }
  for(auto& height : row_max_heights) {
    set_row_height(height.first, height.second);
  }
  for(auto [position, row] : m_visible_rows) {
    row->setGeometry(compute_row_geometry(position));
    row->show();
  }
  update_contents_size();
}

void TableBody::update_visible_region() {
  auto visible_range = compute_visible_range();
  auto new_visible_rows = QSet<int>();
  for(auto row = visible_range.first; row <= visible_range.second; ++row) {
    new_visible_rows.insert(row);
  }
  auto rows_to_remove = QList<int>();
  for(auto i = m_visible_rows.begin(); i != m_visible_rows.end(); ++i) {
    if(!new_visible_rows.contains(i->first)) {
      rows_to_remove.append(i->first);
    }
  }
  for(auto& removals : rows_to_remove) {
    unmount(removals);
  }
  for(auto row = visible_range.first; row <= visible_range.second; ++row) {
    if(!m_visible_rows.contains(row)) {
      mount(row);
    }
  }
  layout_rows();
}

void TableBody::update_column_covers() {
  auto left = 0;
  for(auto column = 0; column != static_cast<int>(m_column_covers.size());
      ++column) {
    auto cover = m_column_covers[column];
    cover->move(left, 0);
    auto width = [&] {
      if(!cover->isVisible()) {
        return 0;
      }
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
}

void TableBody::update_column_widths() {
  for(auto& row : m_visible_rows | std::views::values) {
    if(row->layout()->spacing() != m_styles.m_horizontal_spacing) {
      row->layout()->setSpacing(m_styles.m_horizontal_spacing);
    }
    auto margins = QMargins(m_styles.m_horizontal_spacing, 0,
      m_styles.m_horizontal_spacing, 0);
    if(row->layout()->contentsMargins() != margins) {
      row->layout()->setContentsMargins(margins);
    }
    for(auto column = 0; column != m_widths->get_size(); ++column) {
      auto& item = *row->get_item(column);
      if(m_column_covers[column]->isVisible()) {
        item.setFixedWidth(m_widths->get(column) - get_left_spacing(column));
      } else {
        item.setFixedWidth(0);
      }
    }
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
        previous_item->isAncestorOf(
          static_cast<QWidget*>(QApplication::focusObject()));
      unmatch(*previous_item->parentWidget(), CurrentRow());
      unmatch(*previous_item, Current());
    }
    if(!current || current->m_column != previous->m_column) {
      unmatch(*m_column_covers[previous->m_column], CurrentColumn());
    }
/*
    if(m_current_row && get_layout().indexOf(m_current_row) == -1) {
      destroy(m_current_row);
    }
*/
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
    if(previous_had_focus || QApplication::focusObject() == this) {
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
  update_column_widths();
  update_contents_size();
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
    [&] (const TableModel::StartTransaction&) {
      m_is_transaction = true;
    },
    [&] (const TableModel::EndTransaction&) {
      m_is_transaction = false;
    },
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
  if(!m_is_transaction) {
    update_contents_size();
    update_visible_region();
  }
}

void TableBody::on_widths_update(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      auto spacing = get_left_spacing(operation.m_index);
      for(auto& row : m_visible_rows | std::views::values) {
        if(auto item = row->get_item(operation.m_index)) {
          if(m_column_covers[operation.m_index]->isVisible()) {
            item->setFixedWidth(m_widths->get(operation.m_index) - spacing);
          } else {
            item->setFixedWidth(0);
          }
        }
      }
    });
}
