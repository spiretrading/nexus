#ifndef SPIRE_GROUPED_TABLE_VIEW_HPP
#define SPIRE_GROUPED_TABLE_VIEW_HPP
#include <optional>
#include "Spire/Ui/GroupedTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
namespace Styles {

  /** Selects the rows that display group headings and their aggregates. */
  using AggregateRow = StateSelector<void, struct AggregateRowTag>;
}

  /** Builds a table with expandable group headings. */
  class GroupedTableViewBuilder {
    public:

      /**
       * Starts an ungrouped view with a Group by menu.
       * @param source The table whose rows can be grouped.
       * @param parent The parent widget of the resulting view.
       */
      explicit GroupedTableViewBuilder(
        std::shared_ptr<TableModel> source, QWidget* parent = nullptr);

      /**
       * Adds the next source column.
       * @param name The column header text.
       */
      GroupedTableViewBuilder& add_column(QString name);

      /**
       * Adds the next source column with a compact header.
       * @param name The full column header text.
       * @param short_name The header text to use when space is limited.
       */
      GroupedTableViewBuilder& add_column(QString name, QString short_name);

      /**
       * Sets the initial sort order of the last column added.
       * @param order The sort order, or UNORDERED to disable sorting.
       */
      GroupedTableViewBuilder& set_order(TableHeaderItem::Order order);

      /**
       * Sets how sibling rows are sorted.
       * @param comparator Returns true if the first value precedes the second,
       *        given their grouped row indices and source column index.
       */
      GroupedTableViewBuilder& set_comparator(TableView::Comparator comparator);

      /**
       * Sorts siblings by group key, aggregate, or leaf value.
       * @param comparator Returns true if the first value precedes the second.
       */
      GroupedTableViewBuilder& set_comparator(
        TableView::ValueComparator comparator);

      /**
       * Sets how the last column added is aggregated.
       * @param factory Creates a fresh aggregator for each group, or empty to
       *        use the default aggregator.
       */
      GroupedTableViewBuilder& set_aggregator(
        GroupedTableModel::AggregatorFactory factory);

      /**
       * Sets how aggregate updates are detected across all columns.
       * @param is_equal Returns true if the previous and current aggregates
       *        are equal, or empty to use Spire::is_equal.
       */
      GroupedTableViewBuilder& set_aggregate_equality(
        GroupedTableModel::AggregateEquality is_equal);

      /**
       * Sets the selection model.
       * @param selection The selection in grouped table coordinates.
       */
      GroupedTableViewBuilder& set_selection(
        const std::shared_ptr<TableView::SelectionModel>& selection);

      /**
       * Sets the builder for leaf cells and group keys.
       * @param item_builder Builds widgets using the source table and its row
       *        and column indices.
       */
      GroupedTableViewBuilder& set_item_builder(
        const TableViewItemBuilder& item_builder);

      /**
       * Sets the builder for aggregate cells.
       * @param item_builder Builds widgets using the grouped table, the group
       *        heading's row in that table, and the source column index.
       */
      GroupedTableViewBuilder& set_aggregate_item_builder(
        const TableViewItemBuilder& item_builder);

      /**
       * Fixes the grouping and hides the grouping bar.
       * @param columns Distinct source column indices from outermost to
       *        innermost group, or an empty vector for an ungrouped table.
       */
      GroupedTableViewBuilder& set_grouping(std::vector<int> columns);

      /**
       * Builds the view with one header per source column. If an empty source
       * is populated with a different width, its rows are hidden.
       */
      TableView* make() const;

    private:
      struct Column {
        QString m_name;
        QString m_short_name;
        TableHeaderItem::Order m_order;
        GroupedTableModel::AggregatorFactory m_aggregator;
      };
      std::shared_ptr<TableModel> m_source;
      QWidget* m_parent;
      std::vector<Column> m_columns;
      TableView::Comparator m_comparator;
      GroupedTableModel::AggregateEquality m_is_equal;
      std::optional<std::vector<int>> m_grouping;
      std::shared_ptr<TableView::SelectionModel> m_selection;
      TableViewItemBuilder m_item_builder;
      TableViewItemBuilder m_aggregate_item_builder;
  };
}

#endif
