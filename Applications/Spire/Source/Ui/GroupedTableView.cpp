#include "Spire/Ui/GroupedTableView.hpp"
#include <algorithm>
#include <map>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/EmptyTableFilter.hpp"
#include "Spire/Ui/GroupedTableModel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableGroupingBar.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  class GroupColumns : public QObject {
    public:
      GroupColumns(TableView& view, const GroupedTableModel& table)
          : QObject(&view),
            m_view(view) {
        on_grouping(table.get_grouping());
        m_connection = table.connect_grouping_signal(
          std::bind_front(&GroupColumns::on_grouping, this));
      }

    private:
      struct Column {
        QString m_name;
        QString m_short_name;
        bool m_is_hidden;
      };
      TableView& m_view;
      std::map<int, Column> m_originals;
      scoped_connection m_connection;

      void on_grouping(const std::vector<int>& grouping) {
        auto& header = m_view.get_header();
        auto& items = *header.get_items();
        auto& order = *header.get_column_order();
        for(auto i = 0; i != items.get_size(); ++i) {
          auto column = order.get(i);
          auto grouped = std::ranges::contains(grouping, column);
          auto original = m_originals.find(column);
          if(!grouped && original == m_originals.end()) {
            continue;
          }
          auto& item = items.get(i);
          auto hidden = header.get_item(i)->isHidden();
          if(original == m_originals.end()) {
            original = m_originals.try_emplace(
              column, item.m_name, item.m_short_name, hidden).first;
          }
          auto display = original->second;
          if(grouped) {
            display.m_is_hidden = column != grouping.front();
            if(!display.m_is_hidden) {
              display.m_name = tr("Group");
              display.m_short_name.clear();
            }
          } else {
            m_originals.erase(original);
          }
          if(display.m_name != item.m_name ||
              display.m_short_name != item.m_short_name) {
            auto revised = item;
            revised.m_name = std::move(display.m_name);
            revised.m_short_name = std::move(display.m_short_name);
            items.set(i, revised);
          }
          if(display.m_is_hidden && !hidden) {
            m_view.hide_column(i);
          } else if(!display.m_is_hidden && hidden) {
            m_view.show_column(i);
          }
        }
      }
  };

  class GroupFilter : public EmptyTableFilter {
    public:
      explicit GroupFilter(const GroupedTableModel& table)
          : m_expansions(table.get_row_size()) {
        m_table_connection = table.connect_operation_signal(
          [=, this] (const TableModel::Operation& operation) {
            visit(operation,
              [&] (const TableModel::AddOperation& add) {
                m_expansions.emplace(m_expansions.begin() + add.m_index);
              },
              [&] (const TableModel::RemoveOperation& removal) {
                m_expansions.erase(m_expansions.begin() + removal.m_index);
              },
              [&] (const TableModel::MoveOperation& move) {
                move_element(m_expansions, move.m_source, move.m_destination);
              });
          });
        m_grouping_connection = table.connect_grouping_signal(
          [=, this] (const auto&) {
            m_signal(0, Filter::NONE);
          });
      }

      const std::shared_ptr<LocalBooleanModel>& is_expanded(int row) {
        auto& expansion = m_expansions.at(row);
        if(!expansion) {
          expansion = std::make_unique<Expansion>();
          expansion->m_current = std::make_shared<LocalBooleanModel>(true);
          expansion->m_connection = expansion->m_current->connect_update_signal(
            [=, this] (auto) {
              m_signal(0, Filter::NONE);
            });
        }
        return expansion->m_current;
      }

      bool is_filtered(const TableModel& model, int row) const override {
        auto& table = static_cast<const GroupedTableModel&>(model);
        for(auto i = table.get_parent(row); i != -1; i = table.get_parent(i)) {
          auto& expansion = m_expansions[i];
          if(expansion && !expansion->m_current->get()) {
            return true;
          }
        }
        return false;
      }

      connection connect_filter_signal(
          const FilterSignal::slot_type& slot) const override {
        return m_signal.connect(slot);
      }

    private:
      struct Expansion {
        std::shared_ptr<LocalBooleanModel> m_current;
        scoped_connection m_connection;
      };
      mutable FilterSignal m_signal;
      std::vector<std::unique_ptr<Expansion>> m_expansions;
      scoped_connection m_table_connection;
      scoped_connection m_grouping_connection;
  };

  class GroupCell : public QWidget {
    public:
      GroupCell(const std::shared_ptr<GroupedTableModel>& table, int row,
          int column, GroupFilter& filter) {
        setMinimumHeight(scale_height(28));
        auto layout = make_hbox_layout(this);
        layout->setContentsMargins(scale_width(8), 0, scale_width(8), 0);
        auto label = TableView::default_item_builder(table, row, column);
        auto& grouping = table->get_grouping();
        auto heading = table->index_to_source(row) == -1;
        if(heading) {
          auto palette = this->palette();
          palette.setColor(QPalette::Window, QColor(0, 0, 0, 16));
          setPalette(palette);
          setAutoFillBackground(true);
          if(column == grouping.front()) {
            auto depth = 0;
            for(auto i = table->get_parent(row); i != -1;
                i = table->get_parent(i)) {
              ++depth;
            }
            layout->setContentsMargins(
              scale_width(8 + 16 * depth), 0, scale_width(8), 0);
            static auto closed = image_from_svg(
              ":/Icons/blotter/profit_and_loss/expand_arrow.svg", scale(6, 8));
            static auto open = image_from_svg(
              ":/Icons/blotter/profit_and_loss/collapse_arrow.svg",
              scale(8, 6));
            auto button =
              make_icon_toggle_button(closed, open, filter.is_expanded(row));
            button->setFixedSize(scale(20, 20));
            auto description = [&] {
              auto name = table->get_group_name(row);
              if(name.isEmpty()) {
                return tr("Expand or collapse group");
              }
              return tr("Expand or collapse %1").arg(name);
            }();
            button->setAccessibleName(description);
            button->focusProxy()->setAccessibleName(description);
            layout->addWidget(button);
            setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(button);
          }
        }
        layout->addWidget(label);
        if(!heading) {
          auto on_grouping = [=] (const auto& columns) {
            label->setVisible(columns.empty() || column != columns.front());
          };
          on_grouping(grouping);
          m_connection = table->connect_grouping_signal(on_grouping);
        }
      }

    private:
      scoped_connection m_connection;
  };
}

GroupedTableViewBuilder::GroupedTableViewBuilder(
  std::shared_ptr<TableModel> source, QWidget* parent)
  : m_source(std::move(source)),
    m_parent(parent) {}

GroupedTableViewBuilder& GroupedTableViewBuilder::add_column(QString name) {
  return add_column(std::move(name), QString());
}

GroupedTableViewBuilder& GroupedTableViewBuilder::add_column(
    QString name, QString short_name) {
  m_columns.emplace_back(std::move(name), std::move(short_name),
    TableHeaderItem::Order::NONE, TableAggregationFactory());
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

GroupedTableViewBuilder& GroupedTableViewBuilder::set_aggregation(
    TableAggregationFactory factory) {
  if(m_columns.empty()) {
    throw std::logic_error("Add a column before setting its aggregation.");
  }
  m_columns.back().m_aggregation = std::move(factory);
  return *this;
}

GroupedTableViewBuilder& GroupedTableViewBuilder::set_grouping(
    std::vector<int> columns) {
  m_grouping = std::move(columns);
  return *this;
}

TableView* GroupedTableViewBuilder::make() const {
  if(m_source->get_column_size() != 0 &&
      m_source->get_column_size() != std::ssize(m_columns)) {
    throw std::invalid_argument("Expected a header for each source column.");
  }
  auto factories = std::vector<TableAggregationFactory>();
  for(auto& column : m_columns) {
    factories.push_back(column.m_aggregation);
  }
  auto table = std::make_shared<GroupedTableModel>(
    m_source, m_grouping.value_or(std::vector<int>()), std::move(factories));
  auto filter = std::make_shared<GroupFilter>(*table);
  auto builder = TableViewBuilder(table, m_parent);
  builder.set_sort_handler(
    std::bind_front(&GroupedTableModel::set_column_order, table));
  for(auto& column : m_columns) {
    builder.add_header_item(column.m_name, column.m_short_name, column.m_order,
      TableFilter::Filter::NONE);
  }
  auto view = builder.set_filter(filter).set_item_builder(
    [=] (const auto&, int row, int column) {
      return new GroupCell(table, row, column, *filter);
    }).make();
  new GroupColumns(*view, *table);
  if(!m_grouping) {
    auto names = QStringList();
    for(auto& column : m_columns) {
      names.push_back(column.m_name);
    }
    static_cast<QVBoxLayout*>(view->layout())->insertWidget(
      0, new TableGroupingBar(table, names, view));
  }
  return view;
}
