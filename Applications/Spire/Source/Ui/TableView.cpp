#include "Spire/Ui/TableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QEvent>
#include <QScopedValueRollback>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Spire/TableCurrentIndexModel.hpp"
#include "Spire/Ui/Button.hpp"
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

  bool is_row_change(const TableModel::Operation& operation) {
    return std::holds_alternative<TableModel::AddOperation>(operation) ||
      std::holds_alternative<TableModel::RemoveOperation>(operation) ||
      std::holds_alternative<TableModel::MoveOperation>(operation);
  }

  struct SourceToViewIndexModel : ValueModel<optional<TableIndex>> {
    std::shared_ptr<SortedTableModel> m_sorted_table;
    std::shared_ptr<FilteredTableModel> m_filtered_table;
    std::shared_ptr<TableView::CurrentModel> m_current;
    LocalValueModel<optional<TableIndex>> m_index;
    mutable optional<TableIndex> m_transaction_index;
    int m_source_size;
    bool m_is_transacting;
    scoped_connection m_source_connection;
    scoped_connection m_current_connection;
    scoped_connection m_table_connection;

    SourceToViewIndexModel(std::shared_ptr<SortedTableModel> sorted_table,
        std::shared_ptr<FilteredTableModel> filtered_table,
        std::shared_ptr<TableView::CurrentModel> current)
        : m_sorted_table(std::move(sorted_table)),
          m_filtered_table(std::move(filtered_table)),
          m_current(std::move(current)),
          m_index(from_source(m_current->get())),
          m_source_size(m_filtered_table->get_source()->get_row_size()),
          m_is_transacting(false) {
      m_source_connection =
        m_filtered_table->get_source()->connect_operation_signal(
          std::bind_front(&SourceToViewIndexModel::on_source_operation, this));
      m_current_connection = m_current->connect_update_signal(
        std::bind_front(&SourceToViewIndexModel::on_current, this));
      m_table_connection = m_sorted_table->connect_operation_signal(
        std::bind_front(&SourceToViewIndexModel::on_operation, this));
    }

    const Type& get() const {
      if(m_is_transacting) {
        m_transaction_index = from_source(m_current->get());
        return m_transaction_index;
      }
      return m_index.get();
    }

    QValidator::State test(const Type& value) const {
      return m_current->test(to_source(value));
    }

    QValidator::State set(const Type& value) {
      return m_current->set(to_source(value));
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const {
      return m_index.connect_update_signal(slot);
    }

    optional<TableIndex> from_source(const optional<TableIndex>& index) const {
      if(!index) {
        return none;
      }
      auto filtered = m_filtered_table->index_from_source(index->m_row);
      if(filtered < 0 || filtered >= m_sorted_table->get_row_size()) {
        return none;
      }
      auto row = m_sorted_table->index_from_source(filtered);
      return TableIndex(row, index->m_column);
    }

    optional<TableIndex> to_source(const optional<TableIndex>& index) const {
      if(!index) {
        return none;
      }
      auto row = m_filtered_table->index_to_source(
        m_sorted_table->index_to_source(index->m_row));
      return TableIndex(row, index->m_column);
    }

    void on_current(const optional<TableIndex>& index) {
      if(m_is_transacting ||
          m_source_size != m_filtered_table->get_source()->get_row_size()) {
        return;
      }
      auto displayed = from_source(index);
      if(displayed != m_index.get()) {
        m_index.set(displayed);
      }
    }

    void on_source_operation(const TableModel::Operation& operation) {
      if(std::holds_alternative<TableModel::StartTransaction>(operation)) {
        m_is_transacting = true;
        return;
      } else if(std::holds_alternative<TableModel::EndTransaction>(operation)) {
        m_is_transacting = false;
      } else if(m_is_transacting || !is_row_change(operation)) {
        return;
      }
      m_source_size = m_filtered_table->get_source()->get_row_size();
      on_current(m_current->get());
    }

    void on_operation(const TableModel::Operation& operation) {
      if(is_row_change(operation)) {
        on_current(m_current->get());
      }
    }
  };

  template<typename T>
  class DisplayedSelection : public ArrayListModel<T> {
    public:
      DisplayedSelection(std::shared_ptr<ListModel<T>> source,
          std::shared_ptr<FilteredTableModel> filtered,
          std::shared_ptr<SortedTableModel> sorted, auto is_updating,
          connection tracking)
          : m_source(std::move(source)),
            m_filtered(std::move(filtered)),
            m_sorted(std::move(sorted)),
            m_tracking_connection(std::move(tracking)) {
        m_connection = m_source->connect_operation_signal(
          [=, this] (const auto& operation) {
            if(is_updating()) {
              return;
            }
            if(auto add =
                std::get_if<typename ListModel<T>::AddOperation>(&operation)) {
              if(add->m_index == m_source->get_size() - 1) {
                append(add->m_index);
                return;
              }
            } else if(auto removal = std::get_if<
                typename ListModel<T>::RemoveOperation>(&operation)) {
              if(!m_indices.empty() && removal->m_index == m_indices.back()) {
                m_indices.pop_back();
                ArrayListModel<T>::remove(this->get_size() - 1);
                return;
              }
            }
            if(!std::holds_alternative<
                typename ListModel<T>::PreRemoveOperation>(operation) &&
                !std::holds_alternative<
                  typename ListModel<T>::StartTransaction>(operation) &&
                !std::holds_alternative<
                  typename ListModel<T>::EndTransaction>(operation)) {
              refresh();
            }
          });
        refresh();
      }

      void track(auto refresh) {
        auto on_row_change = [=, this] (const auto& operation) {
          if(is_row_change(operation)) {
            refresh(*this);
          }
        };
        auto& source = *m_filtered->get_source();
        m_source_connection = source.connect_operation_signal(
          [=, this] (const auto& operation) {
            if(auto add = std::get_if<TableModel::AddOperation>(&operation)) {
              if(m_filtered->index_from_source(add->m_index) != -1) {
                return;
              }
            }
            on_row_change(operation);
          });
        m_view_connection = m_sorted->connect_operation_signal(on_row_change);
      }

      void refresh() {
        auto values = std::vector<T>();
        m_indices.clear();
        for(auto i = 0; i != m_source->get_size();) {
          if(auto value = from_source(m_source->get(i))) {
            m_indices.push_back(i);
            values.push_back(*value);
          } else if(m_tracking_connection.connected()) {
            auto blocker = shared_connection_block(m_connection);
            if(m_source->remove(i) != QValidator::Invalid) {
              continue;
            }
          }
          ++i;
        }
        auto first = 0;
        auto end = this->get_size();
        auto next_end = static_cast<int>(values.size());
        while(first < end && first < next_end &&
            this->get(first) == values[first]) {
          ++first;
        }
        while(end > first && next_end > first &&
            this->get(end - 1) == values[next_end - 1]) {
          --end;
          --next_end;
        }
        auto apply = std::function<void ()>([&] {
          if(end == first + 1 && next_end == end) {
            ArrayListModel<T>::set(first, values[first]);
          } else {
            while(end > first) {
              ArrayListModel<T>::remove(--end);
            }
            while(first < next_end) {
              ArrayListModel<T>::insert(values[first], first);
              ++first;
            }
          }
        });
        ArrayListModel<T>::transact(apply);
      }

      QValidator::State set(int index, const T& value) override {
        if(auto translated = to_source(value)) {
          return m_source->set(m_indices.at(index), *translated);
        }
        return QValidator::Invalid;
      }

      QValidator::State push(const T& value) override {
        if(auto translated = to_source(value)) {
          return m_source->push(*translated);
        }
        return QValidator::Invalid;
      }

      QValidator::State insert(const T& value, int index) override {
        if(auto translated = to_source(value)) {
          if(index == this->get_size()) {
            return m_source->insert(*translated, m_source->get_size());
          }
          return m_source->insert(*translated, m_indices.at(index));
        }
        return QValidator::Invalid;
      }

      QValidator::State move(int source, int destination) override {
        if(!m_tracking_connection.connected()) {
          return QValidator::Invalid;
        }
        return m_source->move(m_indices.at(source), m_indices.at(destination));
      }

      QValidator::State remove(int index) override {
        if(!m_tracking_connection.connected()) {
          return QValidator::Invalid;
        }
        return m_source->remove(m_indices.at(index));
      }

    protected:
      void transact(const std::function<void ()>& transaction) override {
        m_source->transact([&] {
          ArrayListModel<T>::transact(transaction);
        });
      }

    private:
      std::shared_ptr<ListModel<T>> m_source;
      std::shared_ptr<FilteredTableModel> m_filtered;
      std::shared_ptr<SortedTableModel> m_sorted;
      std::vector<int> m_indices;
      connection m_tracking_connection;
      scoped_connection m_connection;
      scoped_connection m_source_connection;
      scoped_connection m_view_connection;

      static int& row_of(T& value) {
        if constexpr(std::same_as<T, int>) {
          return value;
        } else {
          return value.m_row;
        }
      }

      optional<T> from_source(T value) const {
        auto& row = row_of(value);
        row = m_filtered->index_from_source(row);
        if(row < 0 || row >= m_sorted->get_row_size()) {
          return none;
        }
        row = m_sorted->index_from_source(row);
        return value;
      }

      void append(int index) {
        if(auto value = from_source(m_source->get(index))) {
          m_indices.push_back(index);
          const auto apply = std::function<void ()>([&] {
            ArrayListModel<T>::insert(*value, this->get_size());
          });
          ArrayListModel<T>::transact(apply);
        } else if(m_tracking_connection.connected()) {
          refresh();
        }
      }

      optional<T> to_source(T value) const {
        auto& row = row_of(value);
        if(!m_tracking_connection.connected() || row < 0 ||
            row >= m_sorted->get_row_size()) {
          return none;
        }
        row = m_filtered->index_to_source(m_sorted->index_to_source(row));
        return value;
      }
  };

  struct TranslatedItemBuilder {
    TableViewItemBuilder m_builder;

    QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column) {
      auto sorted_table = std::static_pointer_cast<SortedTableModel>(table);
      auto filtered_table = std::static_pointer_cast<FilteredTableModel>(
        sorted_table->get_source());
      return m_builder.mount(filtered_table->get_source(),
        filtered_table->index_to_source(sorted_table->index_to_source(row)),
        column);
    }

    void unmount(QWidget* widget) {
      m_builder.unmount(widget);
    }
  };
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
    : QWidget(parent),
      m_table(std::move(table)),
      m_current(std::move(current)),
      m_selection(std::move(selection)),
      m_header(std::move(header)),
      m_filter(std::move(filter)),
      m_horizontal_spacing(0),
      m_vertical_spacing(0) {
  for(auto i = 0; i != m_header->get_size(); ++i) {
    auto& item = m_header->get(i);
    auto filter = m_filter->get_filter(i);
    if(filter != item.m_filter) {
      auto revised_item = item;
      revised_item.m_filter = filter;
      m_header->set(i, revised_item);
    }
  }
  m_header_view = new TableHeader(m_header, m_filter);
  m_header_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  link(*this, *m_header_view);
  auto tracking = m_selection->track(m_table);
  m_filtered_table = std::make_shared<FilteredTableModel>(
    m_table, std::bind_front(&TableFilter::is_filtered, m_filter));
  m_selection_filter_connection = m_selection->m_filter_signal->connect(
    [=, this] (int row, bool& excluded) {
      if(!m_selection->m_is_updating &&
          m_filtered_table->index_from_source(row) == -1) {
        excluded = true;
      }
    });
  if(comparator) {
    m_sorted_table = std::make_shared<SortedTableModel>(
      m_filtered_table, make_column_order(*m_header), std::move(comparator));
  } else {
    m_sorted_table = std::make_shared<SortedTableModel>(
      m_filtered_table, make_column_order(*m_header));
  }
  if(!m_current) {
    m_current = std::make_shared<TableCurrentIndexModel>(m_table);
  }
  auto body_current = make_proxy_value_model(
    std::make_shared<LocalValueModel<optional<Index>>>());
  auto is_updating = [selection = m_selection, tracking] {
    return selection->m_is_updating && tracking.connected();
  };
  auto rows = std::make_shared<DisplayedSelection<int>>(
    m_selection->get_row_selection(), m_filtered_table, m_sorted_table,
    is_updating, tracking);
  auto items = std::make_shared<DisplayedSelection<TableIndex>>(
    m_selection->get_item_selection(), m_filtered_table, m_sorted_table,
    is_updating, tracking);
  auto body_selection = std::make_shared<SelectionModel>(
    items, rows, m_selection->get_column_selection());
  body_selection->m_source_tracking = tracking;
  m_body = new TableBody(m_sorted_table, body_current, body_selection,
    m_header_view->get_widths(),
    TranslatedItemBuilder(std::move(item_builder)));
  auto refresh = [selection = std::weak_ptr(body_selection)] (auto& list) {
    if(auto model = selection.lock()) {
      auto guard = QScopedValueRollback(model->m_is_updating, true);
      list.refresh();
    } else {
      list.refresh();
    }
  };
  rows->track(refresh);
  items->track(refresh);
  body_current->set_source(std::make_shared<SourceToViewIndexModel>(
    m_sorted_table, m_filtered_table, m_current));
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
  m_filter_connection = m_filter->connect_filter_signal(
    std::bind_front(&TableView::on_filter, this));
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&TableView::on_current, this));
  m_body_style_connection = connect_style_signal(
    *m_body, std::bind_front(&TableView::on_body_style, this));
  setFocusProxy(m_scroll_box);
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

void TableView::update_scroll_sizes() {
  auto scroll_size = m_body->estimate_scroll_line_height();
  m_scroll_box->get_vertical_scroll_bar().set_line_size(scroll_size);
  m_scroll_box->get_horizontal_scroll_bar().set_line_size(scroll_size);
}

void TableView::on_order_update(int index, TableHeaderItem::Order order) {
  auto header_item = m_header->get(index);
  header_item.m_order = order;
  auto column_order = m_sorted_table->get_column_order();
  adjust(
    SortedTableModel::ColumnOrder(index, to_table_order(order)), column_order);
  m_sorted_table->set_column_order(column_order);
  for(auto i = 0; i != m_header->get_size(); ++i) {
    if(i == index) {
      m_header->set(i, header_item);
    } else {
      auto item = m_header->get(i);
      if(item.m_order != TableHeaderItem::Order::UNORDERED) {
        item.m_order = TableHeaderItem::Order::NONE;
        m_header->set(i, item);
      }
    }
  }
  m_sort_signal(index, order);
}

void TableView::on_filter(int column, TableFilter::Filter filter) {
  m_filtered_table->set_filter(
    std::bind_front(&TableFilter::is_filtered, m_filter));
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
  m_comparator = comparator;
  return *this;
}

TableViewBuilder& TableViewBuilder::set_comparator(
    TableView::ValueComparator comparator) {
  m_comparator = [comparator = std::move(comparator)] (const AnyRef& left,
      int left_row, const AnyRef& right, int right_row, int column) {
    return comparator(left, right);
  };
  return *this;
}

TableView* TableViewBuilder::make() const {
  return new TableView(m_table, m_header, m_filter, m_current, m_selection,
    m_item_builder, m_comparator, m_parent);
}
