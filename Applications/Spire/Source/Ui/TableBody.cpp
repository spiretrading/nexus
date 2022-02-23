#include "Spire/Ui/TableBody.hpp"
#include <QApplication>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/TextBox.hpp"

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
  return make_label(displayTextAny(table->at(row, column)));
}

struct TableBody::Cover : QWidget {
  QColor m_background_color;

  Cover(QWidget* parent)
    : QWidget(parent),
      m_background_color(Qt::transparent) {}
};

struct TableBody::ColumnCover : Cover {
  QWidget* m_hovered;

  ColumnCover(QWidget* parent)
      : Cover(parent),
        m_hovered(nullptr) {
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
        m_hovered = nullptr;
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
        auto result = QCoreApplication::sendEvent(hovered_widget, &mouse_event);
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

TableBody::TableBody(std::shared_ptr<TableModel> table,
  std::shared_ptr<ListModel<int>> widths, QWidget* parent)
  : TableBody(
      std::move(table), std::make_shared<LocalValueModel<optional<Index>>>(),
      std::move(widths), &default_view_builder, parent) {}

TableBody::TableBody(std::shared_ptr<TableModel> table,
  std::shared_ptr<CurrentModel> current, std::shared_ptr<ListModel<int>> widths,
  QWidget* parent)
  : TableBody(std::move(table), std::move(current), std::move(widths),
      &default_view_builder, parent) {}

TableBody::TableBody(
  std::shared_ptr<TableModel> table, std::shared_ptr<ListModel<int>> widths,
  ViewBuilder view_builder, QWidget* parent)
  : TableBody(std::move(table),
      std::make_shared<LocalValueModel<optional<Index>>>(), std::move(widths),
      std::move(view_builder), parent) {}

TableBody::TableBody(
    std::shared_ptr<TableModel> table, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<ListModel<int>> widths, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_current(std::move(current)),
      m_widths(std::move(widths)),
      m_view_builder(std::move(view_builder)),
      m_current_index(m_current->get()),
      m_current_item(nullptr) {
  setFocusPolicy(Qt::StrongFocus);
  auto row_layout = make_vbox_layout(this);
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableBody::on_style, this));
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_width(1))).
      set(grid_color(QColor(0xE0E0E0)));
    style.get(Any() > Current()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_color(QColor(0x4B23A0)));
    style.get(Any() > CurrentRow()).set(BackgroundColor(QColor(0xE2E0FF)));
    style.get(Any() > CurrentColumn()).set(BackgroundColor(QColor(0xE2E0FF)));
  });
  for(auto row = 0; row != m_table->get_row_size(); ++row) {
    on_table_operation(TableModel::AddOperation(row));
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
  m_current_item = find_item(m_current->get());
  if(m_current_item) {
    match(*m_current_item, Current());
    match(*m_current_item->parentWidget(), CurrentRow());
    match(*m_column_covers[m_current_index->m_column], CurrentColumn());
  }
  m_table_connection = m_table->connect_operation_signal(
    std::bind_front(&TableBody::on_table_operation, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&TableBody::on_current, this));
  m_widths_connection = m_widths->connect_operation_signal(
    std::bind_front(&TableBody::on_widths_update, this));
}

const std::shared_ptr<TableModel>& TableBody::get_table() const {
  return m_table;
}

const std::shared_ptr<TableBody::CurrentModel>& TableBody::get_current() const {
  return m_current;
}

const TableItem* TableBody::get_item(Index index) const {
  return const_cast<TableBody*>(this)->get_item(index);
}

TableItem* TableBody::get_item(Index index) {
  return find_item(index);
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
      cover->setFixedSize(width, height());
      left += width;
    }
    return result;
  } else {
    return QWidget::event(event);
  }
}

void TableBody::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Home:
      if(event->modifiers() & Qt::KeyboardModifier::ControlModifier) {
        navigate_home_row();
      } else {
        navigate_home_column();
      }
      break;
    case Qt::Key_End:
      if(event->modifiers() & Qt::KeyboardModifier::ControlModifier) {
        navigate_end_row();
      } else {
        navigate_end_column();
      }
      break;
    case Qt::Key_Up:
      navigate_previous_row();
      break;
    case Qt::Key_Down:
      navigate_next_row();
      break;
    case Qt::Key_Left:
      navigate_previous_column();
      break;
    case Qt::Key_Right:
      navigate_next_column();
      break;
    default:
      QWidget::keyPressEvent(event);
  }
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
  for(auto i = 0; i != layout()->count(); ++i) {
    auto& cover = *static_cast<Cover*>(layout()->itemAt(i)->widget());
    if(cover.m_background_color.alphaF() != 0 &&
        (!m_current_index || m_current_index->m_row != i)) {
      painter.fillRect(cover.geometry(), cover.m_background_color);
    }
  }
  if(m_current_item) {
    auto column_cover = m_column_covers[m_current_index->m_column];
    if(column_cover->m_background_color.alphaF() != 0) {
      painter.fillRect(
        column_cover->geometry(), column_cover->m_background_color);
    }
    auto& row_cover =
      *static_cast<Cover*>(layout()->itemAt(m_current_index->m_row)->widget());
    if(row_cover.m_background_color.alphaF() != 0) {
      painter.fillRect(row_cover.geometry(), row_cover.m_background_color);
    }
    auto current_position =
      m_current_item->parentWidget()->mapToParent(m_current_item->pos());
    auto& styles = m_current_item->get_styles();
    painter.fillRect(QRect(current_position, m_current_item->size()),
      styles.m_background_color);
  }
  if(m_styles.m_vertical_spacing != 0 &&
      m_styles.m_horizontal_grid_color.alphaF() != 0) {
    auto draw_border = [&] (int top) {
      painter.fillRect(QRect(0, top, width(), m_styles.m_vertical_spacing),
        m_styles.m_horizontal_grid_color);
    };
    auto& row_layout = *static_cast<QBoxLayout*>(layout());
    for(auto row = 0; row != row_layout.count(); ++row) {
      draw_border(
        row_layout.itemAt(row)->geometry().top() - m_styles.m_vertical_spacing);
    }
    draw_border(height() - m_styles.m_vertical_spacing);
  }
  if(m_styles.m_horizontal_spacing != 0 &&
      m_styles.m_vertical_grid_color.alphaF() != 0) {
    auto draw_border = [&] (int left) {
      painter.fillRect(QRect(left, 0, m_styles.m_horizontal_spacing, height()),
        m_styles.m_vertical_grid_color);
    };
    auto left = 0;
    for(auto column = 0; column != m_table->get_column_size(); ++column) {
      draw_border(left);
      if(column != m_widths->get_size()) {
        left += m_widths->get(column);
      }
    }
    draw_border(width() - m_styles.m_horizontal_spacing);
  }
  if(m_current_item) {
    auto current_position =
      m_current_item->parentWidget()->mapToParent(m_current_item->pos());
    auto& styles = m_current_item->get_styles();
    auto top_left = current_position -
      QPoint(m_styles.m_horizontal_spacing, m_styles.m_vertical_spacing);
    auto top_right = top_left + QPoint(
      m_current_item->size().width() + 2 * m_styles.m_horizontal_spacing - 1,
      0);
    auto bottom_right = top_right + QPoint(
      0, m_current_item->size().height() + 2 * m_styles.m_vertical_spacing - 1);
    auto bottom_left = top_left + QPoint(
      0, m_current_item->size().height() + 2 * m_styles.m_vertical_spacing - 1);
    painter.setPen(QPen(QBrush(styles.m_border_top_color),
      m_styles.m_vertical_spacing,  Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.drawLine(QLine(top_left, top_right));
    painter.setPen(QPen(QBrush(styles.m_border_right_color),
      m_styles.m_horizontal_spacing,  Qt::SolidLine, Qt::FlatCap,
      Qt::MiterJoin));
    painter.drawLine(QLine(top_right, bottom_right));
    painter.setPen(QPen(QBrush(styles.m_border_bottom_color),
      m_styles.m_vertical_spacing,  Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.drawLine(bottom_left, bottom_right + QPoint(1, 0));
    painter.setPen(QPen(QBrush(styles.m_border_left_color),
      m_styles.m_horizontal_spacing,  Qt::SolidLine, Qt::FlatCap,
      Qt::MiterJoin));
    painter.drawLine(top_left, bottom_left);
  }
  QWidget::paintEvent(event);
}

TableItem* TableBody::find_item(const optional<Index>& index) {
  if(!index) {
    return nullptr;
  }
  return static_cast<TableItem*>(layout()->itemAt(index->m_row)->widget()->
    layout()->itemAt(index->m_column)->widget());
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

void TableBody::navigate_home() {
  if(m_table->get_row_size() > 0 &&
      m_table->get_column_size() > 0 && m_current->get() != Index(0, 0)) {
    m_current->set(Index(0, 0));
  }
}

void TableBody::navigate_home_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_row != 0) {
    m_current->set(Index(0, current->m_column));
  }
}

void TableBody::navigate_home_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_column != 0) {
    m_current->set(Index(current->m_row, 0));
  }
}

void TableBody::navigate_end() {
  if(m_table->get_row_size() > 0 &&
      m_table->get_column_size() > 0 &&
      m_current->get() !=
        Index(m_table->get_row_size() - 1, m_table->get_column_size() - 1)) {
    m_current->set(
      Index(m_table->get_row_size() - 1, m_table->get_column_size() - 1));
  }
}

void TableBody::navigate_end_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_end();
  } else if(current->m_row != m_table->get_row_size() - 1) {
    m_current->set(Index(m_table->get_row_size() - 1, current->m_column));
  }
}

void TableBody::navigate_end_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_end();
  } else if(current->m_column != m_table->get_column_size() - 1) {
    m_current->set(Index(current->m_row, m_table->get_column_size() - 1));
  }
}

void TableBody::navigate_previous_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_row != 0) {
    m_current->set(Index(current->m_row - 1, current->m_column));
  }
}

void TableBody::navigate_next_row() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_row != m_table->get_row_size() - 1) {
    m_current->set(Index(current->m_row + 1, current->m_column));
  }
}

void TableBody::navigate_previous_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_column != 0) {
    m_current->set(Index(current->m_row, current->m_column - 1));
  }
}

void TableBody::navigate_next_column() {
  auto& current = m_current->get();
  if(!current) {
    navigate_home();
  } else if(current->m_column != m_table->get_column_size() - 1) {
    m_current->set(Index(current->m_row, current->m_column + 1));
  }
}

void TableBody::on_item_clicked(TableItem& item) {
  auto& row_widget = *item.parentWidget();
  auto row_index = layout()->indexOf(&row_widget);
  auto column_index = row_widget.layout()->indexOf(&item);
  if(m_current->get() != Index(row_index, column_index)) {
    m_current->set(Index(row_index, column_index));
  }
}

void TableBody::on_current(const optional<Index>& index) {
  if(index == m_current_index) {
    return;
  }
  auto previous_index = m_current_index;
  auto previous_item = m_current_item;
  m_current_index = index;
  m_current_item = find_item(index);
  if(previous_index) {
    if(previous_item && 
        (!m_current_index || m_current_index->m_row != previous_index->m_row)) {
      unmatch(*previous_item->parentWidget(), CurrentRow());
    }
    if(!m_current_index ||
        m_current_index->m_column != previous_index->m_column) {
      unmatch(*m_column_covers[previous_index->m_column], CurrentColumn());
    }
    if(previous_item) {
      unmatch(*previous_item, Current());
    }
  }
  if(m_current_index) {
    match(*m_current_item, Current());
    if(!previous_index || previous_index->m_row != m_current_index->m_row) {
      match(*m_current_item->parentWidget(), CurrentRow());
    }
    if(!previous_index ||
        previous_index->m_column != m_current_index->m_column) {
      match(*m_column_covers[m_current_index->m_column], CurrentColumn());
    }
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
          m_styles.m_horizontal_spacing = spacing;
        });
      },
      [&] (const VerticalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_styles.m_vertical_spacing = spacing;
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
      });
  }
  auto& row_layout = *layout();
  for(auto row = 0; row != row_layout.count(); ++row) {
    row_layout.itemAt(row)->layout()->setSpacing(m_styles.m_horizontal_spacing);
  }
  row_layout.setSpacing(m_styles.m_vertical_spacing);
  row_layout.setContentsMargins({
    m_styles.m_horizontal_spacing, m_styles.m_vertical_spacing,
    m_styles.m_horizontal_spacing, m_styles.m_vertical_spacing});
  for(auto row = 0; row != row_layout.count(); ++row) {
    auto& column_layout = *row_layout.itemAt(row)->layout();
    for(auto column = 0; column != column_layout.count(); ++column) {
      auto& item = *column_layout.itemAt(column)->widget();
      item.setFixedWidth(m_widths->get(column) - m_styles.m_horizontal_spacing);
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
  auto& row_layout = *static_cast<QBoxLayout*>(layout());
  auto adjusted_current = m_current_index;
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      if(adjusted_current) {
        if(operation.m_index <= adjusted_current->m_row) {
          ++adjusted_current->m_row;
        }
      }
      auto row = new Cover(this);
      match(*row, Row());
      auto column_layout = make_hbox_layout(row);
      column_layout->setSpacing(m_styles.m_horizontal_spacing);
      for(auto column = 0; column != m_table->get_column_size(); ++column) {
        auto item =
          new TableItem(*m_view_builder(m_table, operation.m_index, column));
        if(column != m_table->get_column_size() - 1) {
          item->setFixedWidth(
            m_widths->get(column) - m_styles.m_horizontal_spacing);
        } else {
          item->setSizePolicy(
            QSizePolicy::Expanding, item->sizePolicy().verticalPolicy());
        }
        column_layout->addWidget(item);
        item->connect_clicked_signal(
          std::bind_front(&TableBody::on_item_clicked, this, std::ref(*item)));
      }
      row_layout.insertWidget(operation.m_index, row);
      connect_style_signal(*row, std::bind_front(
        &TableBody::on_cover_style, this, std::ref(*row)));
      on_cover_style(*row);
    },
    [&] (const TableModel::RemoveOperation& operation) {
      if(adjusted_current) {
        if(operation.m_index == adjusted_current->m_row) {
          adjusted_current = none;
        } else if(operation.m_index < adjusted_current->m_row) {
          --adjusted_current->m_row;
        }
      }
      auto row = row_layout.itemAt(operation.m_index);
      row_layout.removeItem(row);
      delete row->widget();
      delete row;
    },
    [&] (const TableModel::MoveOperation& operation) {
      if(adjusted_current) {
        if(adjusted_current->m_row == operation.m_source) {
          adjusted_current->m_row = operation.m_destination;
        } else if(operation.m_source < adjusted_current->m_row &&
            operation.m_destination >= adjusted_current->m_row) {
          --adjusted_current->m_row;
        } else if(operation.m_source > adjusted_current->m_row &&
            operation.m_destination <= adjusted_current->m_row) {
          ++adjusted_current->m_row;
        }
      }
      auto& row = *row_layout.itemAt(operation.m_source);
      row_layout.removeItem(&row);
      auto destination = [&] {
        if(operation.m_source < operation.m_destination) {
          return operation.m_destination - 1;
        }
        return operation.m_destination;
      }();
      row_layout.insertItem(destination, &row);
    });
  if(adjusted_current != m_current_index) {
    if(!adjusted_current) {
      m_current_item = nullptr;
    }
    m_current->set(adjusted_current);
  }
}

void TableBody::on_widths_update(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      auto& row_layout = *layout();
      for(auto i = 0; i != row_layout.count(); ++i) {
        auto& column_layout = *row_layout.itemAt(i)->widget()->layout();
        auto& item = *column_layout.itemAt(operation.m_index)->widget();
        item.setFixedWidth(
          m_widths->get(operation.m_index) - m_styles.m_horizontal_spacing);
      }
    });
}
