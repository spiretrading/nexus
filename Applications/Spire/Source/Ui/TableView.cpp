#include "Spire/Ui/TableView.hpp"
#include <QEvent>
#include <QScopedValueRollback>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/TableCurrentIndexModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Details/DisplayedSelection.hpp"
#include "Spire/Ui/Details/SourceToViewIndexModel.hpp"
#include "Spire/Ui/EmptySelectionModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TableItem.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  class FilteredViewTableModel : public FilteredTableModel {
    public:
      FilteredViewTableModel(std::shared_ptr<TableModel> table,
          const std::shared_ptr<TableFilter>& filter)
          : FilteredTableModel(std::move(table),
              std::bind_front(&TableFilter::is_filtered, filter)) {
        m_connection = filter->connect_filter_signal(
          [=, this] (int, TableFilter::Filter) {
            set_filter(std::bind_front(&TableFilter::is_filtered, filter));
          });
        m_rows_connection = filter->connect_rows_signal(
          std::bind_front(&FilteredTableModel::refresh, this));
      }

    private:
      scoped_connection m_connection;
      scoped_connection m_rows_connection;
  };

  SortedTableModel::Ordering to_table_order(TableHeaderItem::Order order) {
    if(order == TableHeaderItem::Order::ASCENDING) {
      return SortedTableModel::Ordering::ASCENDING;
    } else if(order == TableHeaderItem::Order::DESCENDING) {
      return SortedTableModel::Ordering::DESCENDING;
    }
    return SortedTableModel::Ordering::NONE;
  }

  auto make_column_order(const TableView::HeaderModel& header) {
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    for(auto i = 0; i != header.get_size(); ++i) {
      auto& item = header.get(i);
      if(item.m_order != TableHeaderItem::Order::NONE &&
          item.m_order != TableHeaderItem::Order::UNORDERED) {
        order.push_back(
          SortedTableModel::ColumnOrder(i, to_table_order(item.m_order)));
      }
    }
    return order;
  }
}

QWidget* TableView::default_item_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return TableBody::default_item_builder(table, row, column);
}

TableView::TableView(
    std::shared_ptr<TableModel> table, std::shared_ptr<HeaderModel> header,
    std::shared_ptr<TableFilter> filter, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    TableViewItemBuilder item_builder, Comparator comparator, QWidget* parent)
    : TableView(std::move(table), std::move(header), std::move(filter),
        std::move(current), std::move(selection), std::move(item_builder),
        std::move(comparator), {}, parent) {}

TableView::TableView(std::shared_ptr<TableModel> table,
    std::shared_ptr<HeaderModel> header, std::shared_ptr<TableFilter> filter,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    TableViewItemBuilder item_builder, Comparator comparator,
    SortHandler sort_handler, QWidget* parent)
    : QWidget(parent),
      m_sort(std::move(sort_handler)),
      m_order(make_column_order(*header)),
      m_table(std::move(table)),
      m_current(std::move(current)),
      m_selection(std::move(selection)),
      m_header(std::move(header)),
      m_horizontal_spacing(0),
      m_vertical_spacing(0) {
  for(auto i = 0; i != m_header->get_size(); ++i) {
    auto& item = m_header->get(i);
    auto column_filter = filter->get_filter(i);
    if(column_filter != item.m_filter) {
      auto revised_item = item;
      revised_item.m_filter = column_filter;
      m_header->set(i, revised_item);
    }
  }
  m_header_view = new TableHeader(m_header, filter);
  m_header_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  link(*this, *m_header_view);
  auto tracking = m_selection->track(m_table);
  m_filtered_table = std::make_shared<FilteredViewTableModel>(m_table, filter);
  m_selection_filter_connection = m_selection->m_filter_signal->connect(
    [=, this] (int row, bool& excluded) {
      if(!m_selection->m_is_updating &&
          m_filtered_table->index_from_source(row) == -1) {
        excluded = true;
      }
    });
  if(!m_sort) {
    if(comparator) {
      m_sorted_table = std::make_shared<SortedTableModel>(
        m_filtered_table, m_order, std::move(comparator));
    } else {
      m_sorted_table =
        std::make_shared<SortedTableModel>(m_filtered_table, m_order);
    }
    m_sort =
      std::bind_front(&SortedTableModel::set_column_order, m_sorted_table);
  } else {
    m_sort(m_order);
  }
  initialize_body(std::move(item_builder), std::move(tracking));
  m_body->setSizePolicy(QSizePolicy::MinimumExpanding,
    QSizePolicy::MinimumExpanding);
  m_body->installEventFilter(this);
  link(*this, *m_body);
  m_scroll_box = new ScrollBox(m_body);
  m_scroll_box->set(ScrollBox::DisplayPolicy::ON_ENGAGE);
  m_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_scroll_box->get_horizontal_scroll_bar().connect_position_signal(
    std::bind_front(&TableView::on_scroll_position, this));
  auto layout = make_vbox_layout(this);
  layout->addWidget(m_header_view);
  layout->addWidget(m_scroll_box);
  m_header_view->connect_sort_signal(
    std::bind_front(&TableView::on_order_update, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&TableView::on_current, this));
  m_body_style_connection = connect_style_signal(
    *m_body, std::bind_front(&TableView::on_body_style, this));
  setFocusProxy(m_scroll_box);
  update_style(*m_body, [] (auto& style) {
    auto focused = +Any() < (is_a<TableView>() && FocusIn());
    style.get(Any() > (CurrentColumn() || CurrentRow())).
      set(BackgroundColor(Qt::transparent));
    style.get(Any() > CurrentRow() > Current()).
      set(BackgroundColor(Qt::transparent));
    style.get(CurrentRowOutline() > Row() > Current()).
      set(border_color(QColor(Qt::transparent)));
    auto set_colors = [&] (const Selector& table, QColor outline, QColor fill) {
      style.get(table > Selected()).set(BackgroundColor(fill));
      style.get((table && !CurrentRowOutline()) > Row() > Current()).
        set(border_color(outline));
      style.get((table && CurrentRowOutline()) > CurrentRow()).
        set(grid_color(outline));
    };
    set_colors(Any(), QColor(0xA0A0A0), QColor(0xE0E0E0));
    set_colors(focused, QColor(0x4B23A0), QColor(0xE2E0FF));
  });
  on_body_style();
}

const std::shared_ptr<TableModel>& TableView::get_table() const {
  return m_table;
}

const std::shared_ptr<TableView::CurrentModel>& TableView::get_current() const {
  return m_current;
}

const std::shared_ptr<TableView::SelectionModel>&
    TableView::get_selection() const {
  return m_selection;
}

TableHeader& TableView::get_header() {
  return *m_header_view;
}

TableBody& TableView::get_body() {
  return *m_body;
}

ScrollBox& TableView::get_scroll_box() {
  return *m_scroll_box;
}

void TableView::show_column(int column) {
  if(column < 0 || column >= m_header_view->get_widths()->get_size()) {
    return;
  }
  m_header_view->get_item(column)->show();
  m_body->show_column(column);
}

void TableView::hide_column(int column) {
  if(column < 0 || column >= m_header_view->get_widths()->get_size()) {
    return;
  }
  m_header_view->get_item(column)->hide();
  m_body->hide_column(column);
}

void TableView::set_column_order(
    std::vector<SortedTableModel::ColumnOrder> order) {
  std::erase_if(order, [] (const auto& key) {
    return key.m_order == SortedTableModel::Ordering::NONE;
  });
  m_sort(order);
  m_order = std::move(order);
  auto& columns = *m_header_view->get_column_order();
  for(auto i = 0; i != m_header->get_size(); ++i) {
    auto item = m_header->get(i);
    if(item.m_order == TableHeaderItem::Order::UNORDERED) {
      continue;
    }
    auto direction = TableHeaderItem::Order::NONE;
    if(!m_order.empty() && columns.get(i) == m_order.front().m_index) {
      if(m_order.front().m_order == SortedTableModel::Ordering::ASCENDING) {
        direction = TableHeaderItem::Order::ASCENDING;
      } else {
        direction = TableHeaderItem::Order::DESCENDING;
      }
    }
    if(item.m_order != direction) {
      item.m_order = direction;
      m_header->set(i, item);
    }
  }
}

connection TableView::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

bool TableView::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body) {
    if(event->type() == QEvent::Resize || event->type() == QEvent::Show) {
      auto result = QWidget::eventFilter(watched, event);
      update_scroll_sizes();
      return result;
    }
  }
  return QWidget::eventFilter(watched, event);
}

TableViewItemBuilder TableView::make_displayed_item_builder(
    TableViewItemBuilder item_builder) const {
  struct TranslatedItemBuilder {
    TableViewItemBuilder m_builder;
    Rows m_rows;

    std::optional<Selector> get_row_selector(int row) const {
      return m_builder.get_row_selector(m_rows.index_to_source(row));
    }

    QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      return m_builder.mount(
        m_rows.get_filtered_table()->get_source(),
        m_rows.index_to_source(row), column);
    }

    void unmount(QWidget* widget) {
      m_builder.unmount(widget);
    }
  };
  return TranslatedItemBuilder(
    std::move(item_builder), Rows(m_filtered_table, m_sorted_table));
}

void TableView::initialize_body(
    TableViewItemBuilder item_builder, connection tracking) {
  if(!m_current) {
    m_current = std::make_shared<TableCurrentIndexModel>(m_table);
  }
  auto body_current = make_proxy_value_model(
    std::make_shared<LocalValueModel<optional<Index>>>());
  auto mapping = Rows(m_filtered_table, m_sorted_table);
  auto rows = std::make_shared<DisplayedSelection<int>>(
    m_selection->get_row_selection(), mapping, nullptr, m_selection, tracking);
  auto& column_order = m_header_view->m_translation;
  auto items = std::make_shared<DisplayedSelection<TableIndex>>(
    m_selection->get_item_selection(), mapping, column_order, m_selection,
    tracking);
  auto columns = std::make_shared<DisplayedSelection<int>>(
    m_selection->get_column_selection(), Rows(), column_order, m_selection,
    tracking);
  auto body_selection = std::make_shared<SelectionModel>(items, rows, columns);
  body_selection->m_source_tracking = tracking;
  m_body = new TableBody(mapping.get_table(), body_current, body_selection,
    m_header_view->get_widths(),
    make_displayed_item_builder(std::move(item_builder)));
  auto refresh = [selection = std::weak_ptr(body_selection)] (
      auto& list, const auto&... operation) {
    if(auto model = selection.lock()) {
      auto guard = QScopedValueRollback(model->m_is_updating, true);
      list.refresh(operation...);
    } else {
      list.refresh(operation...);
    }
  };
  rows->track(refresh);
  items->track(refresh);
  columns->track(refresh);
  body_current->set_source(std::make_shared<SourceToViewIndexModel>(
    mapping, column_order, m_current));
}

void TableView::update_scroll_sizes() {
  auto scroll_size = m_body->estimate_scroll_line_height();
  m_scroll_box->get_vertical_scroll_bar().set_line_size(scroll_size);
  m_scroll_box->get_horizontal_scroll_bar().set_line_size(scroll_size);
}

void TableView::on_order_update(int index, TableHeaderItem::Order order) {
  auto source = m_header_view->get_column_order()->get(index);
  auto keys = std::vector<SortedTableModel::ColumnOrder>();
  if(m_sorted_table) {
    keys = m_sorted_table->get_column_order();
  } else {
    keys = m_order;
  }
  adjust(SortedTableModel::ColumnOrder(source, to_table_order(order)), keys);
  set_column_order(std::move(keys));
  m_sort_signal(source, order);
}

void TableView::on_current(const optional<Index>&) {
  auto current = m_body->get_current()->get();
  if(!current) {
    return;
  }
  if(auto item = m_body->find_item(*current)) {
    auto& horizontal_scroll_bar = m_scroll_box->get_horizontal_scroll_bar();
    auto old_x = horizontal_scroll_bar.get_position();
    auto& vertical_scroll_bar = m_scroll_box->get_vertical_scroll_bar();
    auto old_y = vertical_scroll_bar.get_position();
    m_scroll_box->scroll_to(*item);
    auto x = horizontal_scroll_bar.get_position();
    if(x > old_x) {
      horizontal_scroll_bar.set_position(x + m_horizontal_spacing);
    } else if(x < old_x) {
      horizontal_scroll_bar.set_position(x - m_horizontal_spacing);
    }
    auto y = vertical_scroll_bar.get_position();
    if(y > old_y) {
      vertical_scroll_bar.set_position(y + m_vertical_spacing);
    } else if(y < old_y) {
      vertical_scroll_bar.set_position(y - m_vertical_spacing);
    }
  }
}

void TableView::on_body_style() {
  auto& stylist = find_stylist(*m_body);
  m_horizontal_spacing = 0;
  m_vertical_spacing = 0;
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const HorizontalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_horizontal_spacing = spacing;
          update();
        });
      },
      [&] (const VerticalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_vertical_spacing = spacing;
          update();
        });
      });
  }
  update_scroll_sizes();
  update();
}

void TableView::on_scroll_position(int position) {
  m_header_view->get_scroll_box().get_horizontal_scroll_bar().set_position(
    position);
}

TableViewBuilder::TableViewBuilder(
  std::shared_ptr<TableModel> table, QWidget* parent)
  : m_table(table),
    m_parent(parent),
    m_header(std::make_shared<ArrayListModel<TableHeaderItem::Model>>()),
    m_filter(std::make_shared<EmptyTableFilter>()),
    m_selection(std::make_shared<TableSelectionModel>(
      std::make_shared<TableEmptySelectionModel>(),
      std::make_shared<ListSingleSelectionModel>(),
      std::make_shared<ListEmptySelectionModel>())),
    m_item_builder(&TableView::default_item_builder) {}

TableViewBuilder& TableViewBuilder::set_header(
    const std::shared_ptr<TableView::HeaderModel>& header) {
  m_header = header;
  return *this;
}

TableViewBuilder& TableViewBuilder::add_header_item(QString name) {
  return add_header_item(std::move(name), QString());
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, QString short_name) {
  return add_header_item(
    std::move(name), std::move(short_name), TableFilter::Filter::NONE);
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, TableHeaderItem::Order order) {
  return add_header_item(
    std::move(name), QString(), order, TableFilter::Filter::NONE);
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, TableFilter::Filter filter) {
  return add_header_item(std::move(name), QString(), filter);
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, QString short_name, TableFilter::Filter filter) {
  return add_header_item(std::move(name), std::move(short_name),
    TableHeaderItem::Order::NONE, filter);
}

TableViewBuilder& TableViewBuilder::add_header_item(
    QString name, QString short_name, TableHeaderItem::Order order,
    TableFilter::Filter filter) {
  m_header->push(TableHeaderItem::Model(std::move(name), std::move(short_name),
    order, filter));
  return *this;
}

TableViewBuilder& TableViewBuilder::set_filter(
    const std::shared_ptr<TableFilter>& filter) {
  m_filter = filter;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_standard_filter() {
  if(m_table->get_row_size() == 0) {
    return *this;
  }
  auto types = std::vector<std::type_index>();
  for(auto i = 0; i != m_table->get_column_size(); ++i) {
    types.push_back(m_table->at(0, i).get_type());
  }
  return set_filter(std::make_shared<StandardTableFilter>(std::move(types)));
}

TableViewBuilder& TableViewBuilder::set_current(
    const std::shared_ptr<TableView::CurrentModel>& current) {
  m_current = current;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_selection(
    const std::shared_ptr<TableView::SelectionModel>& selection) {
  m_selection = selection;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_item_builder(
    const TableViewItemBuilder& item_builder) {
  m_item_builder = item_builder;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_comparator(
    TableView::Comparator comparator) {
  m_sort = {};
  m_comparator = comparator;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_comparator(
    TableView::ValueComparator comparator) {
  m_sort = {};
  m_comparator = [comparator = std::move(comparator)] (const AnyRef& left,
      int left_row, const AnyRef& right, int right_row, int column) {
    return comparator(left, right);
  };
  return *this;
}

TableViewBuilder& TableViewBuilder::set_sort_handler(
    TableView::SortHandler handler) {
  m_comparator = {};
  m_sort = std::move(handler);
  return *this;
}

TableView* TableViewBuilder::make() const {
  return new TableView(m_table, m_header, m_filter, m_current, m_selection,
    m_item_builder, m_comparator, m_sort, m_parent);
}
