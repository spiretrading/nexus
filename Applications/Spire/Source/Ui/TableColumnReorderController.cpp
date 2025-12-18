#include "Spire/Ui/TableColumnReorderController.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DRAG_DROP_INDICATOR_COLOR = QColor(0x4B23A0);

  std::vector<int> make_default_column_order(int size) {
    auto column_order = std::vector<int>(size);
    std::iota(column_order.begin(), column_order.end(), 0);
    return column_order;
  }

  auto make_header_model(const ListModel<TableHeaderItem::Model>& model,
      int column) {
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

  template <typename Widths>
  std::tuple<bool, int, int> find_column_at_position(TableHeader& header,
      const Widths& widths, int x) {
    auto last_visible_column = -1;
    auto left = 0;
    auto index = 0;
    for(auto i = std::begin(widths); i != std::end(widths); ++i, ++index) {
      auto item = header.get_item(index);
      if(!item->isVisible()) {
        continue;
      }
      last_visible_column = index;
      auto right = left + *i;
      if(x >= left && x < right) {
        return {true, index, left};
      }
      left = right;
    }
    return {false, last_visible_column, left};
  }

  struct ColumnViewTableModel : TableModel {
    std::shared_ptr<TableModel> m_source;
    int m_column;
    TableModelTransactionLog m_transaction;
    scoped_connection m_source_connection;

    ColumnViewTableModel(std::shared_ptr<TableModel> source, int column)
      : m_source(std::move(source)),
        m_column(column),
        m_source_connection(m_source->connect_operation_signal(
          std::bind_front(&ColumnViewTableModel::on_operation, this))) {}

    int get_row_size() const override {
      return m_source->get_row_size();
    }

    int get_column_size() const override {
      return 1;
    }

    AnyRef at(int row, int column) const override {
      if(column != 0) {
        throw std::out_of_range("The column is out of range.");
      }
      return m_source->at(row, m_column);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_transaction.connect_operation_signal(slot);
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::UpdateOperation& operation) {
          if(operation.m_column != m_column) {
            return;
          }
          m_transaction.push(TableModel::UpdateOperation(operation.m_row, 0,
            operation.m_previous, operation.m_value));
        },
        [&] (const auto& operation) {
          m_transaction.push(operation);
        });
    }
  };

  struct ColumnTableViewItemBuilder {
    TableViewItemBuilder m_builder;
    std::shared_ptr<TableModel> m_source;
    int m_column;

    ColumnTableViewItemBuilder(TableViewItemBuilder builder,
      std::shared_ptr<TableModel> source, int column)
      : m_builder(std::move(builder)),
        m_source(std::move(source)),
        m_column(column) {}

    QWidget* mount(const std::shared_ptr<TableModel>& table, int row,
        int column) {
      return m_builder.mount(m_source, row, m_column);
    }

    void unmount(QWidget* widget) {
      m_builder.unmount(widget);
    }
  };

  QWidget* make_column_preview(TableView& table_view,
      const TableViewItemBuilder& m_item_builder,
      int visual_index, int logical_index) {
    auto& table_header = table_view.get_header();
    auto table_view_preview = TableViewBuilder(
      std::make_shared<ColumnViewTableModel>(
        table_view.get_table(), logical_index)).
      set_header(make_header_model(*table_header.get_items(), visual_index)).
      set_item_builder(ColumnTableViewItemBuilder(
        m_item_builder, table_view.get_table(), logical_index)).
      set_current(
        std::make_shared<ConstantValueModel<optional<TableIndex>>>(none)).
      make();
    table_view_preview->get_scroll_box().set(ScrollBox::DisplayPolicy::NEVER);
    auto& float_table_header = table_view_preview->get_header();
    float_table_header.get_widths()->set(0,
      table_header.get_widths()->get(visual_index));
    set_style(*table_view_preview, get_style(table_view));
    set_style(float_table_header, get_style(table_header));
    set_style(*float_table_header.get_item(0),
      get_style(*table_header.get_item(visual_index)));
    set_style(table_view_preview->get_body(), get_style(table_view.get_body()));
    auto preview = new Box(table_view_preview, table_view.window());
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
  TableView& table_view, TableViewItemBuilder item_builder)
  : TableColumnReorderController(table_view, std::move(item_builder),
      make_default_column_order(
        table_view.get_header().get_items()->get_size())) {}

TableColumnReorderController::TableColumnReorderController(
    TableView& table_view, TableViewItemBuilder item_builder,
    std::vector<int> column_order)
    : QObject(&table_view),
      m_table_view(&table_view),
      m_item_builder(std::move(item_builder)),
      m_column_order(std::move(column_order)),
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
  header.setCursor(Qt::OpenHandCursor);
  m_connection = header.get_items()->connect_operation_signal(
    std::bind_front(&TableColumnReorderController::on_operation, this));
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
        start_drag(mouse_event);
      }
    } else if(event->type() == QEvent::MouseButtonRelease) {
      stop_drag();
    } else if(event->type() == QEvent::MouseMove) {
      drag_move(*static_cast<QMouseEvent*>(event));
    }
  }
  return QObject::eventFilter(watched, event);
}

void TableColumnReorderController::start_drag(const QMouseEvent& mouse_event) {
  if(m_column_preview) {
    return;
  }
  auto& header = m_table_view->get_header();
  auto [is_found, index, _] = find_column_at_position(
    header, *header.get_widths(), mouse_event.x());
  if(!is_found) {
    return;
  }
  QApplication::setOverrideCursor(Qt::ClosedHandCursor);
  header.grabMouse();
  auto& header_stylist = find_stylist(header);
  auto header_proxies = header_stylist.get_proxies();
  for(auto& property : header_proxies[0]->get_computed_block()) {
    property.visit(
      [&] (const PaddingLeft& padding) {
        header_stylist.evaluate(padding, [&] (auto padding) {
          m_left_padding = padding;
        });
      });
  }
  header.get_item(index)->installEventFilter(this);
  auto& widths = header.get_widths();
  m_widths.assign(widths->begin(), widths->end());
  m_column_cover = make_column_cover(*m_table_view);
  m_column_cover->setFixedSize(
    widths->get(index) + m_left_padding, m_table_view->height());
  move_column_cover(index);
  m_column_preview = make_column_preview(*m_table_view, m_item_builder,
    index, m_column_order[index]);
  m_column_preview->setFixedSize(m_column_cover->width() + scale_width(1),
    m_column_cover->height() + scale_height(1));
  auto parent = m_column_preview->parentWidget();
  auto position = m_column_cover->mapTo(parent, QPoint(0, 0));
  m_column_preview->move(position.x() - scale_width(1),
    position.y() - scale_height(1));
  auto& horizontal_scroll_bar =
    m_table_view->get_scroll_box().get_horizontal_scroll_bar();
  auto original_horizontal_scroll_bar_position =
    horizontal_scroll_bar.mapToGlobal(QPoint(0, 0));
  if(horizontal_scroll_bar.isVisible()) {
    m_horizontal_scroll_bar_parent =
      raise_scroll_bar(horizontal_scroll_bar, *parent);
  }
  auto& vertical_scroll_bar =
    m_table_view->get_scroll_box().get_vertical_scroll_bar();
  auto original_vertical_scroll_bar_position =
    vertical_scroll_bar.mapToGlobal(QPoint(0, 0));
  if(vertical_scroll_bar.isVisible()) {
    m_vertical_scroll_bar_parent =
      raise_scroll_bar(vertical_scroll_bar, *parent);
  }
  if(horizontal_scroll_bar.isVisible()) {
    horizontal_scroll_bar.move(
      parent->mapFromGlobal(original_horizontal_scroll_bar_position));
  }
  if(vertical_scroll_bar.isVisible()) {
    vertical_scroll_bar.move(
      parent->mapFromGlobal(original_vertical_scroll_bar_position));
  }
  m_preview_x_offset =
    m_column_preview->mapFromGlobal(mouse_event.globalPos()).x();
  m_source_index = index;
  m_current_index = index;
}

void TableColumnReorderController::stop_drag() {
  QApplication::restoreOverrideCursor();
  m_widths.clear();
  if(m_column_preview) {
    m_column_preview->deleteLater();
    m_column_preview = nullptr;
  }
  if(m_column_cover) {
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
  header.releaseMouse();
  if(m_current_index >= 0) {
    header.get_item(m_current_index)->removeEventFilter(this);
    m_column_moved_signal(m_source_index, m_current_index);
    m_source_index = -1;
    m_current_index = -1;
  }
}

void TableColumnReorderController::drag_move(const QMouseEvent& mouse_event) {
  if(!m_column_preview) {
    return;
  }
  if(m_last_mouse_x == mouse_event.pos().x()) {
    return;
  }
  m_last_mouse_x = mouse_event.pos().x();
  auto mouse_x = std::max(0, mouse_event.x());
  auto& header = m_table_view->get_header();
  auto [is_found, index, item_left] =
    find_column_at_position(header, m_widths, mouse_x);
  auto should_move_column = [&] {
    if(is_found) {
      if(index != m_current_index) {
        return true;
      }
    } else if(mouse_x >= item_left && index != m_current_index) {
      return true;
    }
    return false;
  }();
  if(should_move_column) {
    auto blocker = shared_connection_block(m_connection);
    header.get_items()->move(m_current_index, index);
    move_element(m_column_order, m_current_index, index);
    m_current_index = index;
  }
  auto& horizontal_scroll_bar =
    m_table_view->get_scroll_box().get_horizontal_scroll_bar();
  if(horizontal_scroll_bar.isVisible()) {
    update_horizontal_scroll_position(mouse_x);
    if(m_column_cover) {
      move_column_cover(m_current_index);
    }
  }
  auto x = std::max(m_table_view->x(),
    std::min(m_table_view->width() - m_column_preview->width() +
      2 * scale_width(1),
      m_column_preview->parentWidget()->mapFromGlobal(
        mouse_event.globalPos()).x() - m_preview_x_offset));
  m_column_preview->move(x, m_column_preview->y());
}

void TableColumnReorderController::move_column_cover(int index) {
  m_column_cover->move(m_table_view->get_header().get_item(index)->mapTo(
    m_table_view, QPoint(0, 0)).x() - m_left_padding, 0);
}

void TableColumnReorderController::update_horizontal_scroll_position(
    int mouse_x) {
  auto preview_x = std::max(0, mouse_x - m_preview_x_offset);
  auto preview_right = preview_x + m_column_preview->width();
  auto& horizontal_scroll_bar =
    m_table_view->get_scroll_box().get_horizontal_scroll_bar();
  auto scroll_position = horizontal_scroll_bar.get_position();
  auto scroll_right =
    scroll_position + horizontal_scroll_bar.get_page_size();
  if(preview_x < scroll_position) {
    horizontal_scroll_bar.set_position(preview_x);
  } else if(preview_right >= scroll_right) {
    horizontal_scroll_bar.set_position(std::min(
      horizontal_scroll_bar.get_range().m_end,
      preview_right - horizontal_scroll_bar.get_page_size()));
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
