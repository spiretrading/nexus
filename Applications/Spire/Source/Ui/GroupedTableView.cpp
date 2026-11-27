#include "Spire/Ui/GroupedTableView.hpp"
#include <algorithm>
#include <map>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ReadOnlyTableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Spire/TransformListModel.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableGroupingBar.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  enum class HeaderDisplay {
    PLAIN,
    GROUP_COUNT
  };

  struct GroupHeaderItem {
    TableHeaderItem::Model m_header;
    int m_additional_groups;
  };

  auto make_header_model(
      const std::shared_ptr<ArrayListModel<GroupHeaderItem>>& columns,
      HeaderDisplay mode) {
    auto display = [=] (const auto& column) {
      auto header = column.m_header;
      if(mode == HeaderDisplay::GROUP_COUNT &&
          column.m_additional_groups != 0) {
        auto suffix = QObject::tr(" (+%1)").arg(column.m_additional_groups);
        if(!header.m_name.isEmpty()) {
          header.m_name += suffix;
        }
        if(!header.m_short_name.isEmpty()) {
          header.m_short_name += suffix;
        }
      }
      return header;
    };
    auto update = [=] (const auto& column, auto header) {
      auto previous = display(column);
      if(header.m_name == previous.m_name) {
        header.m_name = column.m_header.m_name;
      }
      if(header.m_short_name == previous.m_short_name) {
        header.m_short_name = column.m_header.m_short_name;
      }
      return GroupHeaderItem(std::move(header), column.m_additional_groups);
    };
    using Model = TransformListModel<TableHeaderItem::Model, GroupHeaderItem,
      decltype(display), decltype(update)>;
    return std::make_shared<Model>(columns, display, update);
  }

  class GroupColumns : public QObject {
    public:
      GroupColumns(TableView& view, const GroupedTableModel& table,
          std::shared_ptr<ArrayListModel<GroupHeaderItem>> columns)
          : QObject(&view),
            m_view(view),
            m_columns(std::move(columns)),
            m_group_column(-1) {
        on_grouping(table.get_grouping());
        m_connection = table.connect_grouping_signal(
          std::bind_front(&GroupColumns::on_grouping, this));
      }

    private:
      TableView& m_view;
      std::shared_ptr<ArrayListModel<GroupHeaderItem>> m_columns;
      int m_group_column;
      std::map<int, bool> m_was_hidden;
      scoped_connection m_connection;

      void on_grouping(const std::vector<int>& grouping) {
        auto& header = m_view.get_header();
        auto& items = *header.get_items();
        auto& order = *header.get_column_order();
        auto destination = 0;
        auto source = -1;
        for(auto i = 0; i != items.get_size(); ++i) {
          auto column = order.get(i);
          if(column == m_group_column) {
            destination = i;
          }
          auto additional_groups = 0;
          if(!grouping.empty() && column == grouping.front()) {
            source = i;
            additional_groups = static_cast<int>(grouping.size()) - 1;
          }
          if(m_columns->get(i).m_additional_groups != additional_groups) {
            auto item = m_columns->get(i);
            item.m_additional_groups = additional_groups;
            m_columns->set(i, item);
          }
          auto grouped = std::ranges::contains(grouping, column);
          auto hidden = header.get_item(i)->isHidden();
          auto hide = hidden;
          if(grouped) {
            m_was_hidden.try_emplace(column, hidden);
            hide = column != grouping.front();
          } else if(auto original = m_was_hidden.extract(column)) {
            hide = original.mapped();
          }
          if(hide && !hidden) {
            m_view.hide_column(i);
          } else if(!hide && hidden) {
            m_view.show_column(i);
          }
        }
        if(source == -1) {
          m_group_column = -1;
        } else {
          m_group_column = grouping.front();
          if(source != destination) {
            items.move(source, destination);
          }
        }
      }
  };

  class GroupFilter : public EmptyTableFilter {
    public:
      explicit GroupFilter(const GroupedTableModel& table)
          : m_table(table) {
        m_table_connection = table.connect_operation_signal(
          [=, this] (const TableModel::Operation& operation) {
            visit(operation,
              [&] (const TableModel::AddOperation& add) {
                insert_expansion(add.m_index);
              },
              [&] (const TableModel::RemoveOperation& removal) {
                if(removal.m_index < std::ssize(m_expansions)) {
                  m_expansions.erase(m_expansions.begin() + removal.m_index);
                }
              },
              [&] (const TableModel::MoveOperation& move) {
                move_expansion(move.m_source, move.m_destination);
              });
          });
        m_grouping_connection = table.connect_grouping_signal(
          [=, this] (const auto& columns) {
            if(columns.empty()) {
              m_expansions.clear();
            }
            m_signal(0, Filter::NONE);
          });
      }

      const std::shared_ptr<LocalBooleanModel>& is_expanded(int row) {
        if(row >= std::ssize(m_expansions)) {
          m_expansions.resize(row + 1);
        }
        auto& expansion = m_expansions.at(row);
        if(!expansion) {
          expansion = std::make_unique<Expansion>();
          expansion->m_current = std::make_shared<LocalBooleanModel>(true);
          auto entry = expansion.get();
          expansion->m_connection = expansion->m_current->connect_update_signal(
            [=, this] (auto) mutable {
              if(row >= std::ssize(m_expansions) ||
                  m_expansions[row].get() != entry) {
                auto i = std::ranges::find(
                  m_expansions, entry, &std::unique_ptr<Expansion>::get);
                row = static_cast<int>(i - m_expansions.begin());
              }
              auto first = row + 1;
              m_rows_signal(first, first + m_table.get_descendant_count(row));
            });
        }
        return expansion->m_current;
      }

      bool is_filtered(const TableModel& model, int row) const override {
        auto& table = static_cast<const GroupedTableModel&>(model);
        for(auto i = table.get_parent(row); i != -1; i = table.get_parent(i)) {
          if(i < std::ssize(m_expansions) && m_expansions[i] &&
              !m_expansions[i]->m_current->get()) {
            return true;
          }
        }
        return false;
      }

      connection connect_filter_signal(
          const FilterSignal::slot_type& slot) const override {
        return m_signal.connect(slot);
      }

      connection connect_rows_signal(
          const RowsSignal::slot_type& slot) const override {
        return m_rows_signal.connect(slot);
      }

    private:
      struct Expansion {
        std::shared_ptr<LocalBooleanModel> m_current;
        scoped_connection m_connection;
      };
      mutable FilterSignal m_signal;
      mutable RowsSignal m_rows_signal;
      const GroupedTableModel& m_table;
      std::vector<std::unique_ptr<Expansion>> m_expansions;
      scoped_connection m_table_connection;
      scoped_connection m_grouping_connection;

      void insert_expansion(int row) {
        if(row < std::ssize(m_expansions)) {
          m_expansions.emplace(m_expansions.begin() + row);
        }
      }

      void move_expansion(int source, int destination) {
        if(source >= std::ssize(m_expansions)) {
          insert_expansion(destination);
        } else if(destination < std::ssize(m_expansions)) {
          move_element(m_expansions, source, destination);
        } else {
          auto expansion = std::move(m_expansions[source]);
          m_expansions.erase(m_expansions.begin() + source);
          if(expansion) {
            m_expansions.resize(destination + 1);
            m_expansions.back() = std::move(expansion);
          }
        }
      }
  };

  struct GroupItemBuilder {
    std::shared_ptr<GroupedTableModel> m_table;
    std::shared_ptr<TableModel> m_heading_source;
    std::shared_ptr<GroupFilter> m_filter;
    TableViewItemBuilder m_item_builder;
    TableViewItemBuilder m_aggregate_item_builder;

    std::optional<Selector> get_row_selector(int row) const {
      auto source = m_table->index_to_source(row);
      if(source == -1) {
        if(auto selector = m_aggregate_item_builder.get_row_selector(row)) {
          return AggregateRow() && *selector;
        }
        return AggregateRow();
      }
      return m_item_builder.get_row_selector(source);
    }

    QWidget* mount(const std::shared_ptr<TableModel>& table, int row,
      int column);

    void unmount(QWidget* widget) {
      delete widget;
    }
  };

  class GroupCell : public QWidget {
    public:
      GroupCell(GroupItemBuilder& builder, int row, int column)
          : m_builder(builder.m_item_builder),
            m_item(nullptr) {
        auto& table = builder.m_table;
        setMinimumHeight(scale_height(28));
        auto layout = make_hbox_layout(this);
        layout->setContentsMargins(scale_width(8), 0, scale_width(8), 0);
        auto& grouping = table->get_grouping();
        auto source = table->index_to_source(row);
        if(source == -1 && column == grouping.front()) {
          initialize_heading(
            table, builder.m_heading_source, row, *builder.m_filter, layout);
          return;
        }
        if(source == -1) {
          m_builder = builder.m_aggregate_item_builder;
          m_item = m_builder.mount(table, row, column);
          layout->addWidget(m_item);
          setFocusPolicy(m_item->focusPolicy());
          setFocusProxy(m_item);
        } else if(grouping.empty() || column != grouping.front()) {
          mount_leaf(table, source, column);
        } else {
          m_connection = table->connect_operation_signal(
            std::bind_front(&GroupCell::on_leaf_operation, this, table,
              TableRowIndexTracker(row), column));
        }
      }

      ~GroupCell() override {
        m_connection.disconnect();
        setFocusProxy(nullptr);
        release_item();
      }

    private:
      struct Heading {
        std::shared_ptr<GroupedTableModel> m_table;
        std::shared_ptr<TableModel> m_source;
        QHBoxLayout* m_layout;
        ToggleButton* m_button;
        std::shared_ptr<LocalTextModel> m_count;
        int m_column;
        TableRowIndexTracker m_row;
        TableRowIndexTracker m_leaf;
        QString m_name;
        int m_size;
      };
      TableViewItemBuilder m_builder;
      QWidget* m_item;
      scoped_connection m_connection;

      void mount_leaf(const std::shared_ptr<GroupedTableModel>& table,
          int source, int column) {
        m_item = m_builder.mount(table->get_source(), source, column);
        layout()->addWidget(m_item);
        setFocusPolicy(m_item->focusPolicy());
        setFocusProxy(m_item);
        m_item->show();
        m_connection = table->connect_grouping_signal(
          [=, this] (const auto& columns) {
            m_item->setVisible(columns.empty() || columns.front() != column);
          });
      }

      void release_item() {
        if(m_item) {
          m_item->setParent(nullptr);
          m_builder.unmount(m_item);
          m_item = nullptr;
        }
      }

      void initialize_heading(const std::shared_ptr<GroupedTableModel>& table,
          const std::shared_ptr<TableModel>& source, int row,
          GroupFilter& filter, QHBoxLayout* layout) {
        auto depth = 0;
        for(auto i = table->get_parent(row); i != -1;
            i = table->get_parent(i)) {
          ++depth;
        }
        layout->setContentsMargins(
          scale_width(8 + 16 * depth), 0, scale_width(8), 0);
        static const auto EXPAND_ARROW =
          image_from_svg(":/Icons/expand-arrow.svg", scale(6, 8));
        static const auto COLLAPSE_ARROW =
          image_from_svg(":/Icons/collapse-arrow.svg", scale(8, 6));
        auto button = make_icon_toggle_button(
          EXPAND_ARROW, COLLAPSE_ARROW, filter.is_expanded(row));
        button->setFixedSize(scale(20, 20));
        layout->addWidget(button);
        setFocusPolicy(Qt::StrongFocus);
        setFocusProxy(button);
        auto size = table->get_group_size(row);
        auto count = std::make_shared<LocalTextModel>(
          QStringLiteral("(%1)").arg(size));
        layout->addSpacing(scale_width(4));
        layout->addWidget(make_label(count));
        layout->addStretch();
        auto heading = Heading(table, source, layout, button, std::move(count),
          table->get_grouping()[depth], TableRowIndexTracker(row),
          TableRowIndexTracker(table->get_first_leaf(row)),
          table->get_group_name(row), size);
        update_name(heading);
        bind_item(heading);
        m_connection = table->connect_operation_signal(std::bind_front(
          &GroupCell::on_heading_operation, this, std::move(heading)));
      }

      void update_name(const Heading& heading) {
        auto description = QString();
        if(heading.m_name.isEmpty()) {
          description = tr("Expand or collapse group");
        } else {
          description = tr("Expand or collapse %1").arg(heading.m_name);
        }
        heading.m_button->setAccessibleName(description);
        heading.m_button->focusProxy()->setAccessibleName(description);
      }

      void bind_item(const Heading& heading) {
        release_item();
        auto leaf = heading.m_leaf.get_index();
        if(leaf != -1) {
          m_item = m_builder.mount(heading.m_source,
            heading.m_table->index_to_source(leaf), heading.m_column);
          heading.m_layout->insertWidget(1, m_item);
          m_item->show();
        }
      }

      void on_leaf_operation(const std::shared_ptr<GroupedTableModel>& table,
          TableRowIndexTracker& index, int column,
          const TableModel::Operation& operation) {
        index.update(operation);
        auto row = index.get_index();
        if(row == -1 ||
            !std::holds_alternative<TableModel::EndTransaction>(operation)) {
          return;
        }
        auto& grouping = table->get_grouping();
        if(grouping.empty() || grouping.front() != column) {
          mount_leaf(table, table->index_to_source(row), column);
        }
      }

      void on_heading_operation(
          Heading& heading, const TableModel::Operation& operation) {
        heading.m_row.update(operation);
        heading.m_leaf.update(operation);
        auto row = heading.m_row.get_index();
        if(row == -1 ||
            !std::holds_alternative<TableModel::EndTransaction>(operation)) {
          return;
        }
        auto name = heading.m_table->get_group_name(row);
        if(name != heading.m_name) {
          heading.m_name = std::move(name);
          update_name(heading);
        }
        auto size = heading.m_table->get_group_size(row);
        if(size != heading.m_size) {
          heading.m_size = size;
          heading.m_count->set(QStringLiteral("(%1)").arg(size));
        }
        auto first = heading.m_table->get_first_leaf(row);
        if(heading.m_leaf.get_index() != first) {
          heading.m_leaf.set(first);
          bind_item(heading);
        }
      }
  };

  QWidget* GroupItemBuilder::mount(
      const std::shared_ptr<TableModel>& table, int row, int column) {
    return new GroupCell(*this, row, column);
  }
}

GroupedTableViewBuilder::GroupedTableViewBuilder(
  std::shared_ptr<TableModel> source, QWidget* parent)
  : m_source(std::move(source)),
    m_parent(parent),
    m_item_builder(&TableView::default_item_builder),
    m_aggregate_item_builder(&TableView::default_item_builder) {}

GroupedTableViewBuilder& GroupedTableViewBuilder::add_column(QString name) {
  return add_column(std::move(name), QString());
}

GroupedTableViewBuilder& GroupedTableViewBuilder::add_column(
    QString name, QString short_name) {
  m_columns.emplace_back(std::move(name), std::move(short_name),
    TableHeaderItem::Order::NONE);
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_order(
    TableHeaderItem::Order order) {
  if(m_columns.empty()) {
    throw std::logic_error("Add a column before setting its order.");
  }
  m_columns.back().m_order = order;
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_comparator(
    TableView::Comparator comparator) {
  m_comparator = std::move(comparator);
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_comparator(
    TableView::ValueComparator comparator) {
  return set_comparator([comparator = std::move(comparator)] (
      const AnyRef& left, int, const AnyRef& right, int, int) {
    return comparator(left, right);
  });
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_aggregator(
    GroupedTableModel::AggregatorFactory factory) {
  if(m_columns.empty()) {
    throw std::logic_error("Add a column before setting its aggregator.");
  }
  m_columns.back().m_aggregator = std::move(factory);
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_aggregate_equality(
    GroupedTableModel::AggregateEquality is_equal) {
  m_is_equal = std::move(is_equal);
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_item_builder(
    const TableViewItemBuilder& item_builder) {
  m_item_builder = item_builder;
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_aggregate_item_builder(
    const TableViewItemBuilder& item_builder) {
  m_aggregate_item_builder = item_builder;
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_grouping(
    std::vector<int> columns) {
  m_grouping = std::move(columns);
  return *this;
}

TableView* GroupedTableViewBuilder::make() const {
  auto column_count = static_cast<int>(m_columns.size());
  if((m_source->get_column_size() != 0 || m_source->get_row_size() != 0) &&
      m_source->get_column_size() != column_count) {
    throw std::invalid_argument("Expected a header for each source column.");
  }
  auto factories = std::vector<GroupedTableModel::AggregatorFactory>();
  for(auto& column : m_columns) {
    factories.push_back(column.m_aggregator);
  }
  auto table = std::make_shared<GroupedTableModel>(
    m_source, std::move(factories), m_comparator, m_is_equal);
  table->set_column_count(column_count);
  if(m_grouping) {
    table->set_grouping(*m_grouping);
  }
  auto filter = std::make_shared<GroupFilter>(*table);
  auto builder = TableViewBuilder(table, m_parent);
  builder.set_sort_handler(
    std::bind_front(&GroupedTableModel::set_column_order, table));
  auto columns = std::make_shared<ArrayListModel<GroupHeaderItem>>();
  for(auto& column : m_columns) {
    columns->push(GroupHeaderItem(TableHeaderItem::Model(column.m_name,
      column.m_short_name, column.m_order, TableFilter::Filter::NONE), 0));
  }
  builder.set_header(make_header_model(columns, HeaderDisplay::GROUP_COUNT));
  auto view = builder.set_filter(filter).set_item_builder(GroupItemBuilder(
    table, std::make_shared<ReadOnlyTableModel>(m_source), filter,
    m_item_builder, m_aggregate_item_builder)).make();
  update_style(*view, [] (auto& style) {
    style.get(Any() > is_a<TableBody>() > (AggregateRow() && !Selected())).
      set(BackgroundColor(QColor(0, 0, 0, 16)));
  });
  new GroupColumns(*view, *table, columns);
  if(!m_grouping) {
    auto headers = make_header_model(columns, HeaderDisplay::PLAIN);
    static_cast<QVBoxLayout*>(view->layout())->insertWidget(
      0, new TableGroupingBar(table, headers, view));
  }
  return view;
}
