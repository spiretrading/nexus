#ifndef SPIRE_GROUPED_TABLE_VIEW_HPP
#define SPIRE_GROUPED_TABLE_VIEW_HPP
#include <optional>
#include "Spire/Ui/TableAggregation.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
namespace Styles {

  /** Selects the rows that display group headings and their aggregates. */
  using AggregateRow = StateSelector<void, struct AggregateRowTag>;
}

namespace Details {
  template<typename T>
  concept TableAccumulator = std::is_object_v<T> && requires(T& value) {
    value.add(std::declval<const AnyRef&>());
    value.remove(std::declval<const AnyRef&>());
    value.update(
      std::declval<const AnyRef&>(), std::declval<const AnyRef&>());
    { std::as_const(value).get() } -> std::convertible_to<std::any>;
  };
}

  /** Builds a table with expandable group headings. */
  class GroupedTableViewBuilder {
    public:

      /** Starts an ungrouped view with a Group by menu. */
      explicit GroupedTableViewBuilder(
        std::shared_ptr<TableModel> source, QWidget* parent = nullptr);

      /** Adds a named column in source order. */
      GroupedTableViewBuilder& add_column(QString name);

      /** Adds a column with a short name for compact headers. */
      GroupedTableViewBuilder& add_column(QString name, QString short_name);

      /** Sets the last column's initial order. UNORDERED disables sorting. */
      GroupedTableViewBuilder& set_order(TableHeaderItem::Order order);

      /** Sets the last column's aggregation factory. */
      GroupedTableViewBuilder& set_aggregation(TableAggregationFactory factory);

      /**
       * Sets a factory returning a fresh accumulator with add, remove, and
       * update operations on AnyRefs and a const get returning its value.
       */
      template<std::copy_constructible F> requires
        Details::TableAccumulator<std::invoke_result_t<F&>>
      GroupedTableViewBuilder& set_aggregation(F factory);

      /** Sets the leaf cell builder, which receives source coordinates. */
      GroupedTableViewBuilder& set_item_builder(
        const TableViewItemBuilder& item_builder);

      /**
       * Sets the builder for aggregate cells. It receives the grouped table,
       * the heading's row in that table, and the logical column index.
       */
      GroupedTableViewBuilder& set_aggregate_item_builder(
        const TableViewItemBuilder& item_builder);

      /** Fixes the grouping columns in order and hides the grouping bar. */
      GroupedTableViewBuilder& set_grouping(std::vector<int> columns);

      /** Builds the configured table view. */
      TableView* make() const;

    private:
      struct Column {
        QString m_name;
        QString m_short_name;
        TableHeaderItem::Order m_order;
        TableAggregationFactory m_aggregation;
      };
      std::shared_ptr<TableModel> m_source;
      QWidget* m_parent;
      std::vector<Column> m_columns;
      std::optional<std::vector<int>> m_grouping;
      TableViewItemBuilder m_item_builder;
      TableViewItemBuilder m_aggregate_item_builder;
  };

  template<std::copy_constructible F> requires
    Details::TableAccumulator<std::invoke_result_t<F&>>
  GroupedTableViewBuilder& GroupedTableViewBuilder::set_aggregation(F factory) {
    class Accumulator final : public TableAggregation {
      public:
        explicit Accumulator(F& factory)
          : m_value(std::invoke(factory)) {}

        void add(const AnyRef& value) override {
          m_value.add(value);
        }

        void remove(const AnyRef& value) override {
          m_value.remove(value);
        }

        void update(const AnyRef& previous, const AnyRef& current) override {
          m_value.update(previous, current);
        }

        std::any get() const override {
          return m_value.get();
        }

      private:
        std::invoke_result_t<F&> m_value;
    };
    return set_aggregation([factory = std::move(factory)] () mutable {
      return std::make_unique<Accumulator>(factory);
    });
  }
}

#endif
