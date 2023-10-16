#include "Spire/Ui/TableBody.hpp"
#include <QApplication>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/RowViewListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/ToTextModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TextBox.hpp"

extern bool qt_sendSpontaneousEvent(QObject* receiver, QEvent* event);

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

Spacing Spire::Styles::spacing(int spacing) {
  return Spacing(spacing, spacing);
}

GridColor Spire::Styles::grid_color(QColor color) {
  return GridColor(color, color);
}

QWidget* TableBody::default_view_builder(
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
      m_background_color(Qt::transparent) {}
};

struct TableBody::ColumnCover : Cover {
  QPointer<QWidget> m_hovered;

  ColumnCover(QWidget* parent)
      : Cover(parent) {
    setMouseTracking(true);
  }

  bool event(QEvent* event) override {
    switch(event->type()) {
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseMove:
        return mouse_event(*static_cast<QMouseEvent*>(event));
    }
    return Cover::event(event);
  }

  bool mouse_event(QMouseEvent& event) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    auto hovered_widget = parentWidget()->childAt(mapToParent(event.pos()));
    if(m_hovered != hovered_widget) {
      if(m_hovered) {
        auto leave_event = QEvent(QEvent::Type::Leave);
        QCoreApplication::sendEvent(m_hovered, &leave_event);
      }
      if(hovered_widget == parentWidget()) {
        m_hovered.clear();
      } else {
        m_hovered = hovered_widget;
        if(m_hovered) {
          auto local_position = m_hovered->mapFromGlobal(event.globalPos());
          auto enter_event =
            QEnterEvent(local_position, event.windowPos(), event.screenPos());
          QCoreApplication::sendEvent(m_hovered, &enter_event);
        }
      }
    }
    auto result = [&] {
      if(hovered_widget) {
        auto mouse_event = QMouseEvent(event.type(),
          hovered_widget->mapFromGlobal(event.globalPos()), event.windowPos(),
          event.screenPos(), event.button(), event.buttons(), event.modifiers(),
          event.source());
        auto result = [&] {
          if(event.spontaneous()) {
            return qt_sendSpontaneousEvent(hovered_widget, &mouse_event);
          }
          return QCoreApplication::sendEvent(hovered_widget, &mouse_event);
        }();
        event.setAccepted(mouse_event.isAccepted());
        return result;
      } else {
        return false;
      }
    }();
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    return result;
  }

  void leaveEvent(QEvent* event) override {
    if(m_hovered) {
      auto leave_event = QEvent(QEvent::Type::Leave);
      QCoreApplication::sendEvent(m_hovered, &leave_event);
      m_hovered = nullptr;
    }
    Cover::leaveEvent(event);
  }
};

TableBody::TableBody(
    std::shared_ptr<TableModel> table, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    std::shared_ptr<ListModel<int>> widths, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_current_controller(std::move(current), 0, m_table->get_column_size()),
      m_selection_controller(
        std::move(selection), 0, m_table->get_column_size()),
      m_widths(std::move(widths)),
      m_view_builder(std::move(view_builder)) {
  setAttribute(Qt::WA_Hover);
  setFocusPolicy(Qt::StrongFocus);
  auto row_layout = make_vbox_layout(this);
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
    style.get(Any() > Current()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_color(QColor(0x4B23A0)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(QColor(0xE2E0FF)));
  });
  for(auto row = 0; row != m_table->get_row_size(); ++row) {
    add_row(row);
  }
  auto left = 0;
  for(auto column = 0; column != m_table->get_column_size(); ++column) {
    auto width = [&] {
      if(column != m_widths->get_size()) {
        return m_widths->get(column);
      }
      return this->width() - left;
    }();
    add_column_cover(column, QRect(QPoint(left, 0), QSize(width, height())));
    left += width;
  }
  if(auto current_item = get_current_item()) {
    match(*current_item, Current());
    match(*current_item->parentWidget(), CurrentRow());
    match(*m_column_covers[
      m_current_controller.get_current()->get()->m_column], CurrentColumn());
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

const TableItem* TableBody::get_item(Index index) const {
  return const_cast<TableBody*>(this)->get_item(index);
}

TableItem* TableBody::get_item(Index index) {
  return find_item(index);
}

bool TableBody::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::HoverMove) {
    auto item = static_cast<QWidget*>(watched);
    auto hover_item = find_item(m_hover_index);
    if(hover_item != item) {
      if(hover_item) {
        m_hover_index = none;
        unmatch(*hover_item, HoverItem());
        update();
      }
      if(item != get_current_item()) {
        m_hover_index = [&] () -> optional<Index> {
          for(auto row = 0; row != layout()->count(); ++row) {
            if(auto column =
                layout()->itemAt(row)->widget()->layout()->indexOf(item);
                column >= 0) {
              return Index(row, column);
            }
          }
          return none;
        }();
        match(*item, HoverItem());
      }
    }
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
  } else if(event->type() == QEvent::HoverLeave) {
    if(auto hover_item = find_item(m_hover_index)) {
      m_hover_index = none;
      unmatch(*hover_item, HoverItem());
      update();
    }
    return true;
  } else {
    return QWidget::event(event);
  }
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
    auto& cover = *static_cast<Cover*>(layout()->itemAt(i)->widget());
    if(cover.m_background_color.alphaF() != 0 &&
        (!current || current->m_row != i)) {
      painter.fillRect(cover.geometry(), cover.m_background_color);
    }
  }
  if(auto current_item = get_current_item()) {
    auto column_cover = m_column_covers[current->m_column];
    if(column_cover->m_background_color.alphaF() != 0) {
      painter.fillRect(
        column_cover->geometry(), column_cover->m_background_color);
    }
    auto& row_cover =
      *static_cast<Cover*>(layout()->itemAt(current->m_row)->widget());
    if(row_cover.m_background_color.alphaF() != 0) {
      painter.fillRect(row_cover.geometry(), row_cover.m_background_color);
    }
    auto current_position =
      current_item->parentWidget()->mapToParent(current_item->pos());
    auto& styles = current_item->get_styles();
    painter.fillRect(
      QRect(current_position, current_item->size()), styles.m_background_color);
  }
  if(m_styles.m_horizontal_grid_color.alphaF() != 0) {
    auto draw_border = [&] (int top, int height) {
      painter.fillRect(QRect(0, top, width(), height),
        m_styles.m_horizontal_grid_color);
    };
    if(m_styles.m_padding.top() != 0) {
      draw_border(0, m_styles.m_padding.top());
    }
    if(m_styles.m_vertical_spacing != 0) {
      auto& row_layout = *static_cast<QBoxLayout*>(layout());
      for(auto row = 1; row < row_layout.count(); ++row) {
        draw_border(row_layout.itemAt(row)->geometry().top() -
          m_styles.m_vertical_spacing, m_styles.m_vertical_spacing);
      }
    }
    if(m_styles.m_padding.bottom() != 0) {
      draw_border(height() - m_styles.m_padding.bottom(),
        m_styles.m_padding.bottom());
    }
  }
  if(m_styles.m_vertical_grid_color.alphaF() != 0) {
    auto draw_border = [&] (int left, int width) {
      painter.fillRect(QRect(left, 0, width, height()),
        m_styles.m_vertical_grid_color);
    };
    if(m_styles.m_padding.left() != 0) {
      draw_border(0, m_styles.m_padding.left());
    }
    if(m_styles.m_horizontal_spacing != 0 && m_widths->get_size() > 0) {
      auto left = m_widths->get(0);
      for(auto column = 1; column < m_table->get_column_size(); ++column) {
        draw_border(left, m_styles.m_horizontal_spacing);
        if(column != m_widths->get_size()) {
          left += m_widths->get(column);
        }
      }
    }
    if(m_styles.m_padding.right() != 0) {
      draw_border(width() - m_styles.m_padding.right(),
        m_styles.m_padding.right());
    }
  }
  draw_item_borders(m_hover_index, painter);
  draw_item_borders(current, painter);
  QWidget::paintEvent(event);
}

TableItem* TableBody::get_current_item() {
  return find_item(m_current_controller.get_current()->get());
}

TableBody::Cover* TableBody::find_row(int index) {
  return static_cast<Cover*>(layout()->itemAt(index)->widget());
}

TableItem* TableBody::find_item(const optional<Index>& index) {
  if(!index) {
    return nullptr;
  }
  return static_cast<TableItem*>(layout()->itemAt(index->m_row)->widget()->
    layout()->itemAt(index->m_column)->widget());
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
  connect_style_signal(*cover, std::bind_front(
    &TableBody::on_cover_style, this, std::ref(*cover)));
  cover->show();
  on_cover_style(*cover);
}

void TableBody::add_row(int index) {
  auto row = new Cover(this);
  match(*row, Row());
  auto column_layout = make_hbox_layout(row);
  column_layout->setSpacing(m_styles.m_horizontal_spacing);
  for(auto column = 0; column != m_table->get_column_size(); ++column) {
    auto item = new TableItem(*m_view_builder(m_table, index, column));
    item->setAttribute(Qt::WA_Hover);
    item->installEventFilter(this);
    if(column != m_table->get_column_size() - 1) {
      item->setFixedWidth(m_widths->get(column) - get_left_spacing(column));
    } else {
      item->setSizePolicy(
        QSizePolicy::Expanding, item->sizePolicy().verticalPolicy());
    }
    column_layout->addWidget(item);
    item->connect_active_signal(
      std::bind_front(&TableBody::on_item_activated, this, std::ref(*item)));
  }
  auto& row_layout = *static_cast<QBoxLayout*>(layout());
  row_layout.insertWidget(index, row);
  connect_style_signal(
    *row, std::bind_front(&TableBody::on_cover_style, this, std::ref(*row)));
  on_cover_style(*row);
  m_current_controller.add_row(index);
  m_selection_controller.add_row(index);
}

void TableBody::remove_row(int index) {
  if(m_hover_index && m_hover_index->m_row >= index) {
    unmatch(*find_item(m_hover_index), HoverItem());
    m_hover_index = none;
  }
  auto& row_layout = *static_cast<QBoxLayout*>(layout());
  auto row = row_layout.itemAt(index);
  row_layout.removeItem(row);
  delete row->widget();
  delete row;
  m_current_controller.remove_row(index);
  m_selection_controller.remove_row(index);
}

void TableBody::move_row(int source, int destination) {
  auto& row_layout = *static_cast<QBoxLayout*>(layout());
  auto& row = *row_layout.itemAt(source);
  row_layout.removeItem(&row);
  row_layout.insertItem(destination, &row);
  m_current_controller.move_row(source, destination);
  m_selection_controller.move_row(source, destination);
}

void TableBody::draw_item_borders(const boost::optional<Index>& index,
    QPainter& painter) {
  auto item = find_item(index);
  if(!item) {
    return;
  }
  auto top_spacing = get_top_spacing(index->m_row);
  auto left_spacing = get_left_spacing(index->m_column);
  auto right_spacing = [&] {
    if(index->m_column == m_table->get_column_size() - 1) {
      return m_styles.m_padding.right();
    }
    return m_styles.m_horizontal_spacing;
  }();
  auto bottom_spacing = [&] {
    if(index->m_row == layout()->count() - 1) {
      return m_styles.m_padding.bottom();
    }
    return m_styles.m_vertical_spacing;
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
  painter.fillRect(QRect(left, top, width, top_border_size),
    styles.m_border_top_color);
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
    auto previous_item = find_item(previous);
    unmatch(*previous_item->parentWidget(), CurrentRow());
    if(!current || current->m_column != previous->m_column) {
      unmatch(*m_column_covers[previous->m_column], CurrentColumn());
    }
    unmatch(*previous_item, Current());
  }
  if(current) {
    auto current_item = get_current_item();
    if(current == m_hover_index) {
      unmatch(*current_item, HoverItem());
    }
    match(*current_item, Current());
    match(*current_item->parentWidget(), CurrentRow());
    if(!previous || previous->m_column != current->m_column) {
      match(*m_column_covers[current->m_column], CurrentColumn());
    }
    m_selection_controller.navigate(*current);
  }
}

void TableBody::on_row_selection(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::AddOperation& operation) {
      match(*find_row(operation.get_value()), Selected());
    },
    [&] (const ListModel<int>::UpdateOperation& operation) {
      unmatch(*find_row(operation.get_previous()), Selected());
      match(*find_row(operation.get_value()), Selected());
    });
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
  auto& row_layout = *layout();
  for(auto row = 0; row != row_layout.count(); ++row) {
    row_layout.itemAt(row)->widget()->layout()->setSpacing(
      m_styles.m_horizontal_spacing);
  }
  row_layout.setSpacing(m_styles.m_vertical_spacing);
  row_layout.setContentsMargins(m_styles.m_padding);
  for(auto row = 0; row != row_layout.count(); ++row) {
    auto& column_layout = *row_layout.itemAt(row)->widget()->layout();
    for(auto column = 0; column != m_widths->get_size(); ++column) {
      auto& item = *column_layout.itemAt(column)->widget();
      item.setFixedWidth(m_widths->get(column) - get_left_spacing(column));
    }
  }
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
      auto& row_layout = *layout();
      for(auto i = 0; i != row_layout.count(); ++i) {
        auto& column_layout = *row_layout.itemAt(i)->widget()->layout();
        auto& item = *column_layout.itemAt(operation.m_index)->widget();
        item.setFixedWidth(m_widths->get(operation.m_index) - spacing);
      }
    });
}
