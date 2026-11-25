#ifndef SPIRE_GROUPED_TABLE_VIEW_HPP
#define SPIRE_GROUPED_TABLE_VIEW_HPP
#include "Spire/Ui/TableAggregation.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {

  /** Builds a table with expandable group headings. */
  class GroupedTableViewBuilder {
    public:

      /** Starts a view over the source table. */
      explicit GroupedTableViewBuilder(
        std::shared_ptr<TableModel> source, QWidget* parent = nullptr);

      /** Adds a named column in source order. */
      GroupedTableViewBuilder& add_column(QString name);

      /** Sets the last column's aggregation factory. */
      GroupedTableViewBuilder& set_aggregation(TableAggregationFactory factory);

      /** Sets the grouping columns, from outermost to innermost. */
      GroupedTableViewBuilder& set_grouping(std::vector<int> columns);

      /** Builds the configured table view. */
      TableView* make() const;

    private:
      struct Column {
        QString m_name;
        TableAggregationFactory m_aggregation;
      };
      std::shared_ptr<TableModel> m_source;
      QWidget* m_parent;
      std::vector<Column> m_columns;
      std::vector<int> m_grouping;
  };
}

#endif
