#include "Spire/Ui/TableColumnReorderController.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ColumnViewTableModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DRAG_DROP_INDICATOR_COLOR = QColor(0x4B23A0);

  auto make_header_model(
      const ListModel<TableHeaderItem::Model>& model, int column) {
    auto column_model =
      std::make_shared<ArrayListModel<TableHeaderItem::Model>>();
    column_model->push(model.get(column));
    column_model->push({"", "", TableHeaderItem::Order::UNORDERED,
      TableFilter::Filter::NONE});
    return column_model;
  }

  QWidget* make_column_cover(QWidget& parent) {
    auto cover = new Box(nullptr, &parent);
    update_style(*cover, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(BorderLeftSize(scale_width(2))).
        set(BorderLeftColor(DRAG_DROP_INDICATOR_COLOR));
    });
    cover->show();
    return cover;
  }

  QWidget* raise_scroll_bar(ScrollBar& scroll_bar, QWidget& new_parent) {
    auto parent = scroll_bar.parentWidget();
    parent->layout()->removeWidget(&scroll_bar);
    scroll_bar.setParent(&new_parent);
    scroll_bar.show();
    return parent;
  }

  void restore_scroll_bar(ScrollBar& scroll_bar, QWidget* original_parent,
      int row, int column) {
    static_cast<QGridLayout*>(original_parent->layout())->addWidget(
      &scroll_bar, row, column);
  }

  int find_column_at_position(TableHeader& header, int x) {
    auto last = -1;
    for(auto i = 0; i != header.get_items()->get_size(); ++i) {
      auto item = header.get_item(i);
      if(!item->isVisible()) {
        continue;
      }
      auto left = item->mapTo(&header, QPoint()).x();
      if(x < left) {
        return -1;
      } else if(x < left + item->width()) {
        return i;
      }
      last = i;
    }
    return last;
  }

  QWidget* make_preview(QWidget& column, QWidget& parent) {
    auto preview = new Box(&column, &parent);
    preview->setAttribute(Qt::WA_TransparentForMouseEvents);
    update_style(*preview, [] (auto& style) {
      style.get(Any()).
        set(BorderTopSize(scale_height(1))).
        set(BorderLeftSize(scale_width(1))).
        set(BorderRightSize(scale_width(1))).
        set(border_color(DRAG_DROP_INDICATOR_COLOR));
    });
    auto effect = new QGraphicsOpacityEffect(preview);
    effect->setOpacity(0.5);
    preview->setGraphicsEffect(effect);
    preview->show();
    return preview;
  }

  struct ColumnTableViewItemBuilder {
    TableViewItemBuilder m_builder;
    std::shared_ptr<TableModel> m_source;
    int m_column;

    ColumnTableViewItemBuilder(TableViewItemBuilder builder,
      std::shared_ptr<TableModel> source, int column)
      : m_builder(std::move(builder)),
        m_source(std::move(source)),
        m_column(column) {}

    QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      return m_builder.mount(m_source, row, m_column);
    }

    void unmount(QWidget* widget) {
      m_builder.unmount(widget);
    }
  };

  QWidget* make_column_preview(
      TableView& table_view, const TableViewItemBuilder& m_item_builder,
      int visual_index, int logical_index) {
    auto& table_header = table_view.get_header();
    auto table_view_preview =
      TableViewBuilder(std::make_shared<ColumnViewTableModel>(
        table_view.get_table(), logical_index)).
      set_header(make_header_model(*table_header.get_items(), visual_index)).
      set_item_builder(ColumnTableViewItemBuilder(
        m_item_builder, table_view.get_table(), logical_index)).
      set_current(
        std::make_shared<ConstantValueModel<optional<TableIndex>>>(none)).
      make();
    table_view_preview->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
    auto& float_table_header = table_view_preview->get_header();
    float_table_header.get_widths()->set(
      0, table_header.get_widths()->get(visual_index));
    set_style(*table_view_preview, get_style(table_view));
    set_style(float_table_header, get_style(table_header));
    set_style(*float_table_header.get_item(0),
      get_style(*table_header.get_item(visual_index)));
    set_style(table_view_preview->get_body(), get_style(table_view.get_body()));
    auto preview = make_preview(*table_view_preview, *table_view.window());
    for(auto& selector : find_stylist(table_view).get_matches()) {
      match(*table_view_preview, selector);
    };
    auto& vertical_scroll_bar =
      table_view.get_scroll_box().get_vertical_scroll_bar();
    auto& float_vertical_scroll_bar =
      table_view_preview->get_scroll_box().get_vertical_scroll_bar();
    float_vertical_scroll_bar.set_range(vertical_scroll_bar.get_range());
    float_vertical_scroll_bar.set_position(vertical_scroll_bar.get_position());
    return preview;
  }
}

TableColumnReorderController::TableColumnReorderController(
    TableView& table_view)
    : QObject(&table_view),
      m_table_view(&table_view),
      m_column_order(
        std::from_range, *table_view.get_header().get_column_order()),
      m_column_preview(nullptr),
      m_column_cover(nullptr),
      m_horizontal_scroll_bar_parent(nullptr),
      m_vertical_scroll_bar_parent(nullptr),
      m_last_mouse_x(0),
      m_left_padding(0),
      m_source_index(-1),
      m_current_index(-1),
      m_preview_x_offset(0) {
  auto& header = m_table_view->get_header();
  header.setMouseTracking(true);
  header.installEventFilter(this);
  for(auto i = 0; i != header.get_items()->get_size(); ++i) {
    header.get_item(i)->setCursor(Qt::OpenHandCursor);
  }
  m_connection = header.get_items()->connect_operation_signal(
    std::bind_front(&TableColumnReorderController::on_operation, this));
}

TableColumnReorderController::TableColumnReorderController(
    TableView& table_view, TableViewItemBuilder item_builder)
    : TableColumnReorderController(table_view) {
  m_item_builder.emplace(std::move(item_builder));
}

TableColumnReorderController::TableColumnReorderController(
    TableView& table_view, TableViewItemBuilder item_builder,
    std::vector<int> column_order)
    : TableColumnReorderController(table_view, std::move(item_builder)) {
  m_column_order = std::move(column_order);
}

connection TableColumnReorderController::connect_column_moved_signal(
    const ColumnMoved::slot_type& slot) const {
  return m_column_moved_signal.connect(slot);
}

bool TableColumnReorderController::eventFilter(QObject* watched,
    QEvent* event) {
  if(m_column_cover &&
      watched == m_table_view->get_header().get_item(m_current_index) &&
      event->type() == QEvent::Move) {
    move_column_cover(m_current_index);
  } else if(watched == &m_table_view->get_header()) {
    if(event->type() == QEvent::MouseButtonPress) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      if(mouse_event.button() == Qt::LeftButton) {
        auto& header = m_table_view->get_header();
        auto index = find_column_at_position(header, mouse_event.x());
        if(index != -1 && header.get_item(index)->rect().contains(
            header.get_item(index)->mapFromGlobal(mouse_event.globalPos()))) {
          m_source_index = index;
          m_press_position = mouse_event.globalPos();
        }
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      if(static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
        if(m_column_preview) {
          stop_drag();
        }
        m_source_index = -1;
      }
    } else if(event->type() == QEvent::MouseMove && m_source_index >= 0) {
      auto& mouse = *static_cast<QMouseEvent*>(event);
      if(!m_column_preview && (mouse.buttons() & Qt::LeftButton) &&
          (mouse.globalPos() - m_press_position).manhattanLength() >=
            QApplication::startDragDistance()) {
        start_drag();
      }
      drag_move(mouse);
    } else if(event->type() == QEvent::Hide) {
      if(m_column_preview) {
        stop_drag();
      }
      m_source_index = -1;
    }
  }
  return QObject::eventFilter(watched, event);
}

void TableColumnReorderController::start_drag() {
  auto& header = m_table_view->get_header();
  auto index = m_source_index;
  m_current_index = index;
  m_last_mouse_x = header.mapFromGlobal(m_press_position).x();
  match(*header.get_item(index), Drag());
  QApplication::setOverrideCursor(Qt::ClosedHandCursor);
  if(m_item_builder) {
    auto& header_stylist = find_stylist(header);
    auto header_proxies = header_stylist.get_proxies();
    for(auto& property : header_proxies[0]->get_computed_block()) {
      property.visit([&] (const PaddingLeft& padding) {
        header_stylist.evaluate(padding, [&] (auto padding) {
          m_left_padding = padding;
        });
      });
    }
  }
  auto item = header.get_item(index);
  item->installEventFilter(this);
  auto top = item->mapTo(m_table_view, QPoint(0, 0));
  auto column = QRect(top,
    QSize(item->width() + m_left_padding, m_table_view->height() - top.y()));
  column.translate(-m_left_padding, 0);
  auto& horizontal_scroll_bar =
    m_table_view->get_scroll_box().get_horizontal_scroll_bar();
  if(m_item_builder) {
    m_column_preview = make_column_preview(*m_table_view, *m_item_builder,
      index, m_column_order[index]);
  } else {
    if(horizontal_scroll_bar.isVisible()) {
      auto bottom = horizontal_scroll_bar.mapTo(m_table_view, QPoint()).y();
      column.setBottom(bottom - 1);
    }
    auto label = new QLabel();
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    label->setPixmap(m_table_view->grab(column));
    m_column_preview = make_preview(*label, *m_table_view);
  }
  m_column_cover = make_column_cover(*m_table_view);
  m_column_cover->setGeometry(column);
  m_column_preview->raise();
  m_column_preview->setFixedSize(m_column_cover->width() + scale_width(1),
    m_column_cover->height() + scale_height(1));
  auto parent = m_column_preview->parentWidget();
  m_column_preview->move(m_column_cover->mapTo(parent, QPoint(0, 0)));
  auto original_horizontal_scroll_bar_position =
    horizontal_scroll_bar.mapToGlobal(QPoint(0, 0));
  if(m_item_builder && horizontal_scroll_bar.isVisible()) {
    m_horizontal_scroll_bar_parent =
      raise_scroll_bar(horizontal_scroll_bar, *parent);
  }
  auto& vertical_scroll_bar =
    m_table_view->get_scroll_box().get_vertical_scroll_bar();
  auto original_vertical_scroll_bar_position =
    vertical_scroll_bar.mapToGlobal(QPoint(0, 0));
  if(m_item_builder && vertical_scroll_bar.isVisible()) {
    m_vertical_scroll_bar_parent =
      raise_scroll_bar(vertical_scroll_bar, *parent);
  }
  if(m_horizontal_scroll_bar_parent) {
    horizontal_scroll_bar.move(
      parent->mapFromGlobal(original_horizontal_scroll_bar_position));
  }
  if(m_vertical_scroll_bar_parent) {
    vertical_scroll_bar.move(
      parent->mapFromGlobal(original_vertical_scroll_bar_position));
  }
  m_preview_x_offset =
    m_column_preview->mapFromGlobal(m_press_position).x();
}

void TableColumnReorderController::stop_drag() {
  QApplication::restoreOverrideCursor();
  if(m_column_preview) {
    m_column_preview->hide();
    m_column_preview->deleteLater();
    m_column_preview = nullptr;
  }
  if(m_column_cover) {
    m_column_cover->hide();
    m_column_cover->deleteLater();
    m_column_cover = nullptr;
  }
  if(m_horizontal_scroll_bar_parent) {
    restore_scroll_bar(
      m_table_view->get_scroll_box().get_horizontal_scroll_bar(),
      m_horizontal_scroll_bar_parent, 1, 0);
    m_horizontal_scroll_bar_parent = nullptr;
  }
  if(m_vertical_scroll_bar_parent) {
    restore_scroll_bar(
      m_table_view->get_scroll_box().get_vertical_scroll_bar(),
      m_vertical_scroll_bar_parent, 0, 1);
    m_vertical_scroll_bar_parent = nullptr;
  }
  auto& header = m_table_view->get_header();
  if(m_current_index >= 0) {
    auto item = header.get_item(m_current_index);
    unmatch(*item, Drag());
    item->removeEventFilter(this);
    m_column_moved_signal(m_source_index, m_current_index);
    m_source_index = -1;
    m_current_index = -1;
  }
}

void TableColumnReorderController::drag_move(const QMouseEvent& mouse_event) {
  if(!m_column_preview) {
    return;
  }
  if(m_last_mouse_x == mouse_event.x()) {
    return;
  }
  auto moving_right = mouse_event.x() > m_last_mouse_x;
  m_last_mouse_x = mouse_event.x();
  auto mouse_x = std::max(0, mouse_event.x());
  update_horizontal_scroll_position(mouse_x);
  auto& header = m_table_view->get_header();
  auto index = find_column_at_position(header, mouse_x);
  if(index != -1 && ((moving_right && index > m_current_index) ||
      (!moving_right && index < m_current_index))) {
    auto blocker = shared_connection_block(m_connection);
    header.get_items()->move(m_current_index, index);
    move_element(m_column_order, m_current_index, index);
    m_current_index = index;
  }
  move_column_cover(m_current_index);
  auto parent = m_column_preview->parentWidget();
  auto left = m_table_view->mapTo(parent, QPoint(0, 0)).x();
  auto x = std::clamp(parent->mapFromGlobal(
    mouse_event.globalPos()).x() - m_preview_x_offset, left,
    left + std::max(0, m_table_view->width() - m_column_preview->width()));
  m_column_preview->move(x, m_column_preview->y());
}

void TableColumnReorderController::move_column_cover(int index) {
  m_column_cover->move(m_table_view->get_header().get_item(index)->mapTo(
    m_table_view, QPoint(0, 0)) - QPoint(m_left_padding, 0));
}

void TableColumnReorderController::update_horizontal_scroll_position(
    int mouse_x) {
  auto left = mouse_x - m_preview_x_offset;
  auto right = left + m_column_preview->width();
  auto& horizontal_scroll_bar =
    m_table_view->get_scroll_box().get_horizontal_scroll_bar();
  auto scroll_position = horizontal_scroll_bar.get_position();
  if(left < 0) {
    horizontal_scroll_bar.set_position(scroll_position + left);
  } else if(right > horizontal_scroll_bar.get_page_size()) {
    horizontal_scroll_bar.set_position(
      scroll_position + right - horizontal_scroll_bar.get_page_size());
  }
}

void TableColumnReorderController::on_operation(
    const ListModel<TableHeaderItem::Model>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<TableHeaderItem::Model>::MoveOperation& operation) {
      move_element(m_column_order, operation.m_source,
        operation.m_destination);
    });
}
