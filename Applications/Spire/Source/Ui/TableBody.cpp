#include "Spire/Ui/TableBody.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FixedHorizontalLayout.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto OPERATION_THRESHOLD = 10;
  const auto SCROLL_BUFFER = 1;

  bool test_visibility(const QWidget& container, const QRect& geometry) {
    auto widget_geometry =
      QRect(container.mapToParent(geometry.topLeft()), geometry.size());
    return std::max(-SCROLL_BUFFER, widget_geometry.top()) <=
      std::min(container.parentWidget()->height() + SCROLL_BUFFER,
        widget_geometry.bottom());
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

  struct TableValueToTextModel : ValueModel<QString> {
    std::shared_ptr<TableModel> m_table;
    TableRowIndexTracker m_row;
    int m_column;
    LocalValueModel<QString> m_current;
    scoped_connection m_connection;

    TableValueToTextModel(
        std::shared_ptr<TableModel> table, int row, int column)
        : m_table(std::move(table)),
          m_row(row),
          m_column(column),
          m_current(to_text(m_table->at(m_row.get_index(), column))) {
      m_connection = m_table->connect_operation_signal(
        std::bind_front(&TableValueToTextModel::on_operation, this));
    }

    const Type& get() const override {
      return m_current.get();
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_current.connect_update_signal(slot);
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::UpdateOperation& operation) {
          if(m_row.get_index() == operation.m_row &&
              m_column == operation.m_column) {
            m_current.set(to_text(operation.m_value));
          }
        },
        [&] (const auto& operation) {
          m_row.update(operation);
          if(m_row.get_index() == -1) {
            m_table = nullptr;
            m_connection.disconnect();
          }
        });
    }
  };
}

Spacing Spire::Styles::spacing(int spacing) {
  return Spacing(spacing, spacing);
}

GridColor Spire::Styles::grid_color(QColor color) {
  return GridColor(color, color);
}

QWidget* TableBody::default_item_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return make_label(
    std::make_shared<TableValueToTextModel>(table, row, column));
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
  bool m_is_pending_layout;

  RowCover(TableBody& body)
      : Cover(&body),
        m_is_pending_layout(false) {
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
      get_item(i)->mount(*body.m_item_builder.mount(
        body.m_table, index, body.m_visual_to_logical_columns[i]));
    }
  }

  void unmount() {
    move(-10000, -10000);
    auto& body = *static_cast<TableBody*>(parentWidget());
    for(auto i = 0; i != layout()->count(); ++i) {
      if(auto item = get_item(i)) {
        body.m_item_builder.unmount(item->unmount());
      }
    }
    unmatch(*this, CurrentRow());
    unmatch(*this, Selected());
    if(!testAttribute(Qt::WA_WState_Hidden)) {
      setAttribute(Qt::WA_WState_Hidden);
    }
  }

  void move_column(int source, int destination) {
    static_cast<FixedHorizontalLayout*>(this->layout())->move(
      source, destination);
  }

  QSize sizeHint() const override {
    return layout()->sizeHint();
  }
};

struct TableBody::Layout : QLayout {
  std::vector<int> m_top;
  int m_top_height;
  std::vector<int> m_bottom;
  int m_bottom_height;
  mutable optional<QSize> m_size_hint;
  std::vector<std::unique_ptr<QLayoutItem>> m_items;

  explicit Layout(TableBody& parent)
    : QLayout(&parent),
      m_top_height(0),
      m_bottom_height(0) {}

  int get_top_index() const {
    if(m_top.empty() && m_bottom.empty() && isEmpty()) {
      return -1;
    }
    return static_cast<int>(m_top.size());
  }

  bool is_visible(int index) const {
    return index >= get_top_index() && index < get_top_index() + count();
  }

  int get_top_space() const {
    auto& styles = static_cast<TableBody*>(parent())->m_styles;
    return styles.m_padding.top() + m_top_height +
      static_cast<int>(m_top.size()) * styles.m_vertical_spacing;
  }

  int get_bottom_space() const {
    auto& styles = static_cast<TableBody*>(parent())->m_styles;
    return styles.m_padding.bottom() + m_bottom_height +
      static_cast<int>(m_bottom.size()) * styles.m_vertical_spacing;
  }

  const RowCover& get_row(int index) const {
    return *static_cast<const RowCover*>(itemAt(index)->widget());
  }

  RowCover& get_row(int index) {
    return *static_cast<RowCover*>(itemAt(index)->widget());
  }

  int get_row_height(int row) const {
    if(get_top_index() == -1) {
      return 0;
    } else if(row < static_cast<int>(m_top.size())) {
      return m_top[row];
    } else if(row < static_cast<int>(m_top.size()) + count()) {
      return m_items[row - static_cast<int>(m_top.size())]->sizeHint().height();
    } else {
      return m_bottom[row - static_cast<int>(m_top.size()) - count()];
    }
  }

  void hide(QLayoutItem& item) {
    auto height = item.widget()->sizeHint().height();
    if(&item == m_items.front().get()) {
      m_top_height += height;
      m_top.push_back(height);
      m_items.erase(m_items.begin());
    } else {
      m_bottom_height += height;
      m_bottom.insert(m_bottom.begin(), height);
      std::erase_if(m_items, [&] (const auto& i) {
        return i.get() == &item;
      });
    }
  }

  void add_hidden_row(int index, int height) {
    if(get_top_index() == -1) {
      return;
    }
    if(index < get_top_index()) {
      m_top.insert(m_top.begin() + index, height);
      m_top_height += height;
    } else {
      m_bottom.insert(
        m_bottom.begin() + (index - count() - get_top_index()), height);
      m_bottom_height += height;
    }
  }

  void add_hidden_row(int index) {
    if(get_top_index() == -1) {
      return;
    }
    if(index < get_top_index()) {
      add_hidden_row(index, m_top_height / get_top_index());
    } else if(!m_bottom.empty()) {
      add_hidden_row(
        index, m_bottom_height / static_cast<int>(m_bottom.size()));
    } else if(m_top_height + m_bottom_height != 0) {
      auto average_row_height = (m_top_height + m_bottom_height) /
        static_cast<int>(get_top_index() + m_bottom.size());
      add_hidden_row(index, average_row_height);
    } else {
      add_hidden_row(index, sizeHint().height() / count());
    }
  }

  void remove_hidden_row(int index) {
    if(index < get_top_index()) {
      auto height = m_top[index];
      m_top.erase(m_top.begin() + index);
      m_top_height -= height;
    } else {
      auto i = index - count() - get_top_index();
      if(i < std::ssize(m_bottom)) {
        auto height = m_bottom[i];
        m_bottom.erase(m_bottom.begin() + i);
        m_bottom_height -= height;
      }
    }
  }

  void insert(QLayoutItem& item, int index) {
    m_items.insert(
      m_items.begin() + index, std::unique_ptr<QLayoutItem>(&item));
    item.widget()->show();
  }

  void insert(RowCover& row, int index) {
    if(index == get_top_index() - 1) {
      m_top_height -= m_top.back();
      m_top.pop_back();
    } else if(!m_bottom.empty() && index == get_top_index() + count()) {
      m_bottom_height -= m_bottom.front();
      m_bottom.erase(m_bottom.begin());
    }
    m_items.insert(m_items.begin() + (index - m_top.size()),
      std::make_unique<QWidgetItem>(&row));
    row.m_is_pending_layout = true;
    row.show();
    row.m_is_pending_layout = true;
  }

  void move_row(int source, int destination) {
    auto top = get_top_index();
    auto source_iterator = m_items.begin() + (source - top);
    auto destination_iterator = m_items.begin() + (destination - top);
    if(source < destination) {
      std::rotate(
        source_iterator, source_iterator + 1, destination_iterator + 1);
    } else if(destination < source) {
      std::rotate(destination_iterator, source_iterator, source_iterator + 1);
    }
  }

  void reset_top_index(int top_point) {
    auto size = static_cast<int>(m_top.size() + m_bottom.size());
    if(size == 0) {
      return;
    }
    if(get_top_space() < top_point) {
      while(!m_bottom.empty() && get_top_space() < top_point) {
        auto height = m_bottom.front();
        m_bottom_height -= height;
        m_bottom.erase(m_bottom.begin());
        m_top.push_back(height);
        m_top_height += height;
      }
    } else {
      while(!m_top.empty() && get_top_space() > top_point &&
          get_top_space() - m_top.back() > top_point) {
        auto height = m_top.back();
        m_top_height -= height;
        m_top.pop_back();
        m_bottom.insert(m_bottom.begin(), height);
        m_bottom_height += height;
      }
    }
  }

  void set_row_size(int size) {
    if(isEmpty()) {
      return;
    }
    activate();
    auto additional_rows = size - get_top_index() - count();
    auto& styles = static_cast<TableBody*>(parent())->m_styles;
    auto total_height = itemAt(count() - 1)->geometry().bottom() +
      styles.m_vertical_spacing - count() * styles.m_vertical_spacing;
    auto row_height = total_height / count();
    for(auto i = 0; i != additional_rows; ++i) {
      m_bottom.push_back(row_height);
      m_bottom_height += row_height;
    }
  }

  void addItem(QLayoutItem* item) override {
    m_items.emplace_back(item);
    item->widget()->show();
  }

  QSize sizeHint() const override {
    if(m_size_hint) {
      return *m_size_hint;
    }
    auto margins = contentsMargins();
    auto& styles = static_cast<TableBody*>(parent())->m_styles;
    auto column_size = static_cast<TableBody*>(parent())->get_column_size();
    auto width = margins.left() + margins.right() + 
      (column_size + 1) * styles.m_horizontal_spacing;
    auto height = get_top_space() + get_bottom_space() +
      (count() + 1) * styles.m_vertical_spacing;
    for(auto& item : m_items) {
      width = std::max(width, item->sizeHint().width());
      height += item->sizeHint().height();
    }
    m_size_hint = QSize(width, height);
    return *m_size_hint;
  }

  void setGeometry(const QRect& rect) override {
    QLayout::setGeometry(rect);
    auto contents_rect = contentsRect();
    auto& styles = static_cast<TableBody*>(parent())->m_styles;
    auto y = get_top_space() + styles.m_vertical_spacing;
    auto width = contents_rect.width();
    for(auto& item : m_items) {
      auto row_height = item->sizeHint().height();
      auto geometry = QRect(contents_rect.x(), y, width, row_height);
      if(geometry != item->geometry()) {
        item->setGeometry(geometry);
      }
      y += row_height + styles.m_vertical_spacing;
      static_cast<RowCover*>(item->widget())->m_is_pending_layout = false;
    }
  }

  QLayoutItem* itemAt(int index) const override {
    if(index >= 0 && index < count()) {
      return m_items[index].get();
    }
    return nullptr;
  }

  QLayoutItem* takeAt(int index) override {
    if(index < count()) {
      auto item = std::move(m_items[index]);
      m_items.erase(m_items.begin() + index);
      return item.release();
    }
    return nullptr;
  }

  int count() const override {
    return static_cast<int>(m_items.size());
  }

  void invalidate() override {
    QLayout::invalidate();
    m_size_hint = none;
    for(auto& item : m_items) {
      item->invalidate();
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
    auto paint_border = [&] (int top, const QColor& color) {
      painter.fillRect(QRect(body.m_styles.m_padding.left(), top, width,
        body.m_styles.m_vertical_spacing), color);
    };
    if(body.m_styles.m_vertical_spacing != 0) {
      for(auto i = 0; i != body.get_layout().count(); ++i) {
        auto color = [&] {
          if(i == 0) {
            if(auto row_cover = body.find_row(0)) {
              return row_cover->m_background_color;
            }
          }
          return body.m_styles.m_horizontal_grid_color;
        }();
        auto& row = body.get_layout().get_row(i);
        paint_border(row.y() - body.m_styles.m_vertical_spacing, color);
      }
      if(body.get_layout().count() > 0) {
        auto& row = body.get_layout().get_row(body.get_layout().count() - 1);
        paint_border(row.y() + row.height(),
          body.m_styles.m_horizontal_grid_color);
      }
    }
  }

  static void paint_vertical_borders(TableBody& body, QPainter& painter) {
    if(body.m_styles.m_vertical_grid_color.alphaF() == 0) {
      return;
    }
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
  }

  static void paint_current_item(
      TableBody& body, QPainter& painter, const optional<TableIndex>& current) {
    if(!current) {
      return;
    }
    auto current_item = body.find_item(*current);
    if(!current_item || !current_item->isVisible()) {
      return;
    }
    auto column_cover = body.m_column_covers[current->m_column];
    if(column_cover->m_background_color.alphaF() != 0) {
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
      m_is_transaction(false),
      m_operation_counter(0),
      m_resize_guard(0),
      m_key_observer(*this) {
  setLayout(new Layout(*this));
  setFocusPolicy(Qt::StrongFocus);
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableBody::on_style, this));
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_width(1))).
      set(grid_color(QColor(0xE0E0E0)));
  });
  for(auto row = 0; row != m_table->get_row_size(); ++row) {
    add_row(row);
  }
  m_operation_counter = 0;
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
    m_visual_to_logical_columns.push_back(column);
  }
  if(auto current = m_current_controller.get_column()) {
    match(*m_column_covers[*current], CurrentColumn());
  }
  m_table_connection = m_table->connect_operation_signal(
    std::bind_front(&TableBody::on_table_operation, this));
  m_current_connection =
    m_current_controller.get_current()->connect_update_signal(
     std::bind_front(&TableBody::on_current, this));
  m_selection_connection = m_selection_controller.connect_row_operation_signal(
    std::bind_front(&TableBody::on_row_selection, this));
  m_widths_connection = m_widths->connect_operation_signal(
    std::bind_front(&TableBody::on_widths_update, this));
  m_key_observer.connect_filtered_key_press_signal(
    std::bind_front(&TableBody::on_key_press, this));
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
    if(row == m_current_row &&
        !get_layout().is_visible(*get_current_row_index())) {
      auto position = m_styles.m_padding.top() +
        *get_current_row_index() * estimate_row_height();
      row->move(m_styles.m_padding.left(), position);
    }
    return row->get_item(index.m_column);
  }
  return nullptr;
}

optional<int> TableBody::get_current_row_index() const {
  if(m_current_index) {
    return m_current_index->m_row;
  }
  return none;
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

void TableBody::focusInEvent(QFocusEvent* event) {
  if(event->reason() == Qt::TabFocusReason) {
    focusNextPrevChild(true);
  } else if(event->reason() == Qt::BacktabFocusReason) {
    focusNextPrevChild(false);
  } else {
    QWidget::focusInEvent(event);
  }
}

bool TableBody::focusNextPrevChild(bool next) {
  if(isEnabled()) {
    auto focus_widget = focusWidget();
    if(focus_widget && !focus_widget->isVisible()) {
      setFocus();
      return true;
    }
    if(next) {
      if(navigate_next()) {
        return true;
      }
    } else if(navigate_previous()) {
      return true;
    }
  }
  m_current_controller.get_current()->set(none);
  setFocus();
  return QWidget::focusNextPrevChild(next);
}

void TableBody::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Home:
      if(event->modifiers() & Qt::KeyboardModifier::ControlModifier) {
        m_current_controller.navigate_home_row();
      } else {
        m_current_controller.navigate_home_column();
      }
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
      break;
    case Qt::Key_End:
      if(event->modifiers() & Qt::KeyboardModifier::ControlModifier) {
        m_current_controller.navigate_end_row();
      } else {
        m_current_controller.navigate_end_column();
      }
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
      break;
    case Qt::Key_Up:
      m_current_controller.navigate_previous_row();
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
      break;
    case Qt::Key_Down:
      m_current_controller.navigate_next_row();
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
      break;
    case Qt::Key_Left:
      m_current_controller.navigate_previous_column();
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
      break;
    case Qt::Key_Right:
      m_current_controller.navigate_next_column();
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
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
        (!m_current_index || m_current_index->m_column != i)) {
      painter.fillRect(cover->geometry(), cover->m_background_color);
    }
  }
  for(auto i = 0; i != get_layout().count(); ++i) {
    auto& cover = get_layout().get_row(i);
    auto index = get_layout().get_top_index() + i;
    if(cover.m_background_color.alphaF() != 0 &&
        (!m_current_index || m_current_index->m_row != index)) {
      painter.fillRect(cover.geometry(), cover.m_background_color);
    }
  }
  Painter::paint_current_item(*this, painter, m_current_index);
  Painter::paint_horizontal_borders(*this, painter);
  Painter::paint_vertical_borders(*this, painter);
  Painter::paint_item_borders(*this, painter, m_hover_index);
  Painter::paint_item_borders(*this, painter, m_current_index);
  QWidget::paintEvent(event);
}

void TableBody::resizeEvent(QResizeEvent* event) {
  ++m_resize_guard;
  if(m_resize_guard == 1) {
    update_visible_region();
  }
  --m_resize_guard;
}

void TableBody::showEvent(QShowEvent* event) {
  update_parent();
  update_column_covers();
  update_column_widths();
}

const TableBody::Layout& TableBody::get_layout() const {
  return *static_cast<Layout*>(layout());
}

TableBody::Layout& TableBody::get_layout() {
  return *static_cast<Layout*>(layout());
}

TableBody::RowCover* TableBody::find_row(int index) {
  if(get_layout().is_visible(index)) {
    return &get_layout().get_row(index - get_layout().get_top_index());
  } else if(index == get_current_row_index()) {
    return m_current_row;
  }
  return nullptr;
}

TableItem* TableBody::find_item(const optional<Index>& index) {
  if(index) {
    return find_item(*index);
  }
  return nullptr;
}

TableBody::Index TableBody::get_index(const TableItem& item) const {
  auto row = item.parentWidget();
  if(row == m_current_row) {
    return Index(*get_current_row_index(),
      row->layout()->indexOf(&const_cast<TableItem&>(item)));
  }
  auto row_index = get_layout().get_top_index();
  for(auto i = 0; i != get_layout().count(); ++i) {
    if(row == &get_layout().get_row(i)) {
      break;
    }
    ++row_index;
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

void TableBody::increment_operation_counter() {
  ++m_operation_counter;
  if(m_operation_counter == OPERATION_THRESHOLD) {
    while(!get_layout().isEmpty()) {
      auto item = get_layout().itemAt(0);
      auto row = static_cast<RowCover*>(item->widget());
      get_layout().hide(*item);
      if(row != m_current_row) {
        destroy(row);
      } else {
        row->move(-1000, -1000);
      }
    }
  }
}

void TableBody::add_row(int index) {
  increment_operation_counter();
  if(index <= get_current_row_index().get_value_or(-1)) {
    ++m_current_index->m_row;
  }
  if(get_layout().is_visible(index)) {
    mount_row(index);
  } else {
    get_layout().add_hidden_row(index);
  }
  m_current_controller.add_row(index);
  m_selection_controller.add_row(index);
}

void TableBody::pre_remove_row(int index) {
  increment_operation_counter();
  if(get_layout().is_visible(index)) {
    remove(*find_row(index));
  } else {
    if(index == get_current_row_index()) {
      auto current_row = m_current_row;
      m_current_row = nullptr;
      m_current_index = none;
      destroy(current_row);
    }
    get_layout().remove_hidden_row(index);
  }
  if(index < get_current_row_index().get_value_or(-1)) {
    --m_current_index->m_row;
  }
  if(m_hover_index && m_hover_index->m_row >= index) {
    m_hover_index = none;
  }
}

void TableBody::remove_row(int index) {
  m_current_controller.remove_row(index);
  auto block = shared_connection_block(m_selection_connection);
  m_selection_controller.remove_row(index);
}

void TableBody::move_row(int source, int destination) {
  increment_operation_counter();
  if(m_current_index) {
    auto& current_row = m_current_index->m_row;
    auto direction = source < destination ? -1 : 1;
    if(current_row == source) {
      current_row = destination;
    } else if((current_row - destination) * direction >= 0 &&
        (current_row - source) * direction < 0) {
      current_row += direction;
    }
  }
  auto& layout = get_layout();
  if(layout.is_visible(source)) {
    if(layout.is_visible(destination)) {
      layout.move_row(source, destination);
    } else {
      auto item = layout.itemAt(source - layout.get_top_index());
      layout.add_hidden_row(destination, item->sizeHint().height());
      if(item->widget() == m_current_row) {
        layout.removeItem(item);
        delete item;
      } else {
        remove(*static_cast<RowCover*>(item->widget()));
      }
    }
  } else if(layout.is_visible(destination)) {
    mount_row(destination);
    if(destination < source) {
      ++source;
    }
    layout.remove_hidden_row(source);
  } else {
    auto height = layout.get_row_height(source);
    layout.remove_hidden_row(source);
    if(source < destination) {
      --destination;
    }
    layout.add_hidden_row(destination, height);
  }
  m_selection_controller.move_row(source, destination);
}

void TableBody::update_parent() {
  if(auto parent = parentWidget()) {
    parent->installEventFilter(this);
  }
  initialize_visible_region();
}

TableBody::RowCover* TableBody::mount_row(int index) {
  auto row = [&] {
    if(index == get_current_row_index()) {
      return m_current_row;
    }
    return make_row_cover();
  }();
  if(row != m_current_row) {
    row->mount(index);
  }
  get_layout().insert(*row, index);
  if(row != m_current_row) {
    on_cover_style(*row);
  }
  return row;
}

TableBody::RowCover* TableBody::make_row_cover() {
  if(m_recycled_rows.empty()) {
    auto row = new RowCover(*this);
    connect_style_signal(
      *row, std::bind_front(&TableBody::on_cover_style, this, std::ref(*row)));
    return row;
  }
  auto row = m_recycled_rows.front();
  m_recycled_rows.pop_front();
  row->m_is_pending_layout = false;
  row->layout()->setSpacing(m_styles.m_horizontal_spacing);
  row->layout()->setContentsMargins(
    m_styles.m_horizontal_spacing, 0, m_styles.m_horizontal_spacing, 0);
  for(auto i = 0; i != m_widths->get_size(); ++i) {
    auto spacing = get_left_spacing(i);
    if(auto item = row->get_item(i)) {
      if(m_column_covers[i]->isVisible()) {
        item->setFixedWidth(m_widths->get(i) - spacing);
      } else {
        item->setFixedWidth(0);
      }
    }
  }
  return row;
}

void TableBody::destroy(RowCover* row) {
  row->unmount();
  m_recycled_rows.push_back(row);
}

void TableBody::remove(RowCover& row) {
  auto item = get_layout().takeAt(get_layout().indexOf(&row));
  if(&row == m_current_row) {
    m_current_row = nullptr;
    m_current_index = none;
  }
  destroy(&row);
  delete item;
}

void TableBody::mount_visible_rows() {
  auto top = mapFromParent(QPoint(0, 0)).y() - SCROLL_BUFFER;
  while(get_layout().get_top_index() - 1 >= 0 &&
      get_layout().get_top_space() > top) {
    mount_row(get_layout().get_top_index() - 1);
  }
  auto position = [&] {
    if(get_layout().isEmpty()) {
      return get_layout().get_top_space() + m_styles.m_padding.top();
    }
    auto position = get_layout().get_top_space();
    for(auto i = 0; i != get_layout().count(); ++i) {
      position += get_layout().get_row(i).sizeHint().height() +
        m_styles.m_vertical_spacing;
    }
    return position + 1;
  }();
  auto bottom =
    mapFromParent(QPoint(0, parentWidget()->height())).y() + SCROLL_BUFFER;
  while(get_layout().get_top_index() + get_layout().count() <
      m_table->get_row_size() && position < bottom) {
    auto row = mount_row(get_layout().get_top_index() + get_layout().count());
    position += row->sizeHint().height() + m_styles.m_vertical_spacing;
  }
}

void TableBody::unmount_hidden_rows() {
  auto i = 0;
  while(i != get_layout().count()) {
    auto item = get_layout().itemAt(i);
    auto row = static_cast<RowCover*>(item->widget());
    if(!test_visibility(*this, item->geometry()) && !row->m_is_pending_layout) {
      get_layout().hide(*item);
      if(row != m_current_row) {
        destroy(row);
      } else {
        row->move(-1000, -1000);
      }
    } else {
      ++i;
    }
  }
}

void TableBody::initialize_visible_region() {
  if(!parentWidget() || get_layout().get_top_index() != -1 || !isVisible() ||
      m_table->get_row_size() == 0 || m_table->get_column_size() == 0) {
    return;
  }
  if(m_current_controller.get_row() && !m_current_row) {
    m_current_index = m_current_controller.get();
    m_current_row = make_row_cover();
    m_current_row->mount(m_current_index->m_row);
    on_cover_style(*m_current_row);
  }
  mount_row(0);
  mount_visible_rows();
  get_layout().set_row_size(m_table->get_row_size());
  get_layout().invalidate();
}

void TableBody::reset_visible_region() {
  if(m_table->get_row_size() == 0) {
    return;
  }
  get_layout().reset_top_index(mapFromParent(QPoint(0, 0)).y());
  if(get_layout().get_top_index() < m_table->get_row_size()) {
    mount_row(get_layout().get_top_index());
  }
}

void TableBody::update_visible_region() {
  if(get_layout().get_top_index() == -1) {
    initialize_visible_region();
    return;
  }
  if(!parentWidget() || !isVisible()) {
    return;
  }
  ++m_resize_guard;
  auto are_updates_enabled = updatesEnabled();
  setUpdatesEnabled(false);
  unmount_hidden_rows();
  if(get_layout().isEmpty()) {
    reset_visible_region();
  }
  mount_visible_rows();
  get_layout().setGeometry(get_layout().geometry());
  setUpdatesEnabled(are_updates_enabled);
  --m_resize_guard;
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
  for(auto i = 0; i != get_layout().count() + 1; ++i) {
    auto row = [&] () -> RowCover* {
      if(i == get_layout().count()) {
        return m_current_row;
      }
      auto row = &get_layout().get_row(i);
      if(row != m_current_row) {
        return row;
      }
      return nullptr;
    }();
    if(row) {
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
}

bool TableBody::navigate_next() {
  auto get_next_column =
    [&] (const CurrentModel& current, int row, int column) {
      ++column;
      while(column < get_table()->get_column_size() &&
          !current.test(Index(row, column))) {
        ++column;
      }
      return column;
    };
  if(auto& current = m_current_controller.get()) {
    auto column =
      get_next_column(*get_current(), current->m_row, current->m_column);
    if(column >= get_table()->get_column_size()) {
      auto row = current->m_row + 1;
      if(row >= get_table()->get_row_size()) {
        return false;
      } else {
        get_current()->set(
          Index(row, get_next_column(*get_current(), row, -1)));
        if(auto current = m_current_controller.get()) {
          m_selection_controller.navigate(*current);
        }
      }
    } else {
      get_current()->set(Index(current->m_row, column));
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
    }
  } else if(get_table()->get_row_size() > 0) {
    get_current()->set(Index(0, get_next_column(*get_current(), 0, -1)));
    if(auto current = m_current_controller.get()) {
      m_selection_controller.navigate(*current);
    }
  } else {
    return false;
  }
  return focus_current(*get_current(), *this, Qt::TabFocusReason);
}

bool TableBody::navigate_previous() {
  auto get_previous_column =
    [] (const CurrentModel& current, int row, int column) {
      --column;
      while(column >= 0 && !current.test(Index(row, column))) {
        --column;
      }
      return column;
    };
  if(auto& current = m_current_controller.get()) {
    auto column =
      get_previous_column(*get_current(), current->m_row, current->m_column);
    if(column < 0) {
      auto row = current->m_row - 1;
      if(row < 0) {
        return false;
      } else {
        get_current()->set(Index(row, get_previous_column(*get_current(),
          row, get_table()->get_column_size())));
        if(auto current = m_current_controller.get()) {
          m_selection_controller.navigate(*current);
        }
      }
    } else {
      get_current()->set(Index(current->m_row, column));
      if(auto current = m_current_controller.get()) {
        m_selection_controller.navigate(*current);
      }
    }
  } else if(get_table()->get_row_size() > 0) {
    auto row = get_table()->get_row_size() - 1;
    auto column =
      get_previous_column(*get_current(), row, get_table()->get_column_size());
    get_current()->set(Index(row, column));
    if(auto current = m_current_controller.get()) {
      m_selection_controller.navigate(*current);
    }
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

void TableBody::on_current(const optional<Index>& current) {
  if(current == m_current_index) {
    return;
  }
  auto previous = m_current_index;
  auto previous_had_focus = false;
  if(m_current_index) {
    if(auto previous_item = find_item(*m_current_index)) {
      previous_had_focus =
        previous_item->isAncestorOf(
          static_cast<QWidget*>(QApplication::focusObject()));
      if(!current || m_current_index->m_row != current->m_row) {
        unmatch(*previous_item->parentWidget(), CurrentRow());
      }
      unmatch(*previous_item, Current());
    }
    if(!current || current->m_column != m_current_index->m_column) {
      unmatch(*m_column_covers[m_current_index->m_column], CurrentColumn());
    }
    if(get_layout().indexOf(m_current_row) == -1) {
      destroy(m_current_row);
    }
    m_current_row = nullptr;
    m_current_index = none;
  }
  if(m_operation_counter >= OPERATION_THRESHOLD) {
    if(!current && previous) {
      m_selection_controller.remove_row(previous->m_row);
    }
    return;
  }
  if(current) {
    m_current_index = current;
    if(get_layout().is_visible(m_current_index->m_row)) {
      m_current_row = static_cast<RowCover*>(get_layout().itemAt(
        m_current_index->m_row - get_layout().get_top_index())->widget());
    } else {
      m_current_row = make_row_cover();
      m_current_row->mount(m_current_index->m_row);
      on_cover_style(*m_current_row);
    }
    auto current_item = m_current_row->get_item(m_current_index->m_column);
    match(*current_item, Current());
    match(*current_item->parentWidget(), CurrentRow());
    if(!previous || previous->m_column != m_current_index->m_column) {
      match(*m_column_covers[m_current_index->m_column], CurrentColumn());
    }
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
    [&] (const ListModel<int>::PreRemoveOperation& operation) {
      auto& selection =
        m_selection_controller.get_selection()->get_row_selection();
      if(auto row = find_row(selection->get(operation.m_index))) {
        unmatch(*row, Selected());
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
  if(get_layout().contentsMargins() != m_styles.m_padding) {
    get_layout().setContentsMargins(m_styles.m_padding);
  }
  update_column_widths();
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
      m_operation_counter = 0;
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
    if(m_operation_counter >= OPERATION_THRESHOLD) {
      m_operation_counter = 0;
      on_current(m_current_controller.get());
    } else {
      m_operation_counter = 0;
    }
    update_visible_region();
  }
}

void TableBody::on_widths_update(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      auto spacing = get_left_spacing(operation.m_index);
      for(auto i = 0; i != get_layout().count() + 1; ++i) {
        auto row = [&] () -> RowCover* {
          if(i == get_layout().count()) {
            return m_current_row;
          }
          auto row = &get_layout().get_row(i);
          if(row != m_current_row) {
            return row;
          }
          return nullptr;
        }();
        if(row) {
          if(auto item = row->get_item(operation.m_index)) {
            if(m_column_covers[operation.m_index]->isVisible()) {
              item->setFixedWidth(m_widths->get(operation.m_index) - spacing);
            } else {
              item->setFixedWidth(0);
            }
          }
        }
      }
    },
    [&] (const ListModel<int>::MoveOperation& operation) {
      if(operation.m_source == operation.m_destination) {
        return;
      }
      move_element(m_visual_to_logical_columns, operation.m_source,
        operation.m_destination);
      move_element(m_column_covers, operation.m_source,
        operation.m_destination);
      auto& layout = get_layout();
      for(auto i = 0; i < layout.count(); ++i) {
        layout.get_row(i).move_column(operation.m_source,
          operation.m_destination);
      }
      for(auto& row_cover : m_recycled_rows) {
        row_cover->move_column(operation.m_source, operation.m_destination);
      }
    });
}

bool TableBody::on_key_press(QWidget& target, QKeyEvent& event) {
  if(auto scroll_box = dynamic_cast<ScrollBox*>(&target)) {
    auto key = event.key();
    if(key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up ||
        key == Qt::Key_Down || key == Qt::Key_Home || key == Qt::Key_End ||
        key == Qt::Key_PageUp || key == Qt::Key_PageDown) {
      event.ignore();
      return true;
    }
  }
  return false;
}
