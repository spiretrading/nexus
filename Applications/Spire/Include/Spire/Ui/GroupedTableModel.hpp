#ifndef SPIRE_GROUPED_TABLE_MODEL_HPP
#define SPIRE_GROUPED_TABLE_MODEL_HPP
#include <map>
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TransactionLogModel.hpp"
#include "Spire/Ui/TableAggregation.hpp"

namespace Spire {

  /**
   * Groups rows of a source table by one or more columns and displays aggregate
   * values in each group heading.
   */
  class GroupedTableModel : public TransactionLogModel<TableModel> {
    public:

      /** Signals the new grouping columns after regrouping. */
      using GroupingSignal = Signal<void (const std::vector<int>&)>;

      /**
       * Groups a table using the default column aggregations.
       * @param source The table containing the leaves.
       * @param columns Distinct column indices, from outermost to innermost.
       *        An empty list preserves the source layout.
       */
      GroupedTableModel(
        std::shared_ptr<TableModel> source, std::vector<int> columns);

      /**
       * Groups a table with custom column aggregations.
       * @param source The table containing the leaves.
       * @param columns The grouping columns, from outermost to innermost.
       * @param aggregations Factories indexed by source column. Omitted or
       *        empty entries use the default aggregation.
       */
      GroupedTableModel(std::shared_ptr<TableModel> source,
        std::vector<int> columns,
        std::vector<TableAggregationFactory> aggregations);

      ~GroupedTableModel() override;

      /** Returns the table containing the leaves. */
      const std::shared_ptr<TableModel>& get_source() const;

      /** Returns the grouping columns, from outermost to innermost. */
      const std::vector<int>& get_grouping() const;

      /** Sets the grouping columns. An empty list restores source order. */
      void set_grouping(std::vector<int> columns);

      /** Connects a slot to changes in the grouping columns. */
      boost::signals2::connection connect_grouping_signal(
        const GroupingSignal::slot_type& slot) const;

      /** Returns the parent heading's row, or -1 for a top-level row. */
      int get_parent(int row) const;

      /** Returns the source index, or -1 for a heading or an invalid row. */
      int index_to_source(int row) const;

      /** Returns the grouped index of a source row, or -1 if absent. */
      int index_from_source(int row) const;

      int get_row_size() const override;
      int get_column_size() const override;
      AnyRef at(int row, int column) const override;
      QValidator::State set(
        int row, int column, const std::any& value) override;
      QValidator::State remove(int row) override;

    private:
      struct Group;
      struct Row;
      using Changes = std::map<int, std::any>;
      mutable GroupingSignal m_grouping_signal;
      std::shared_ptr<TableModel> m_source;
      std::vector<int> m_columns;
      std::vector<TableAggregationFactory> m_aggregations;
      std::vector<Row> m_rows;
      std::vector<int> m_indices;
      std::vector<std::pair<std::shared_ptr<Group>, int>> m_dirty;
      std::map<int, Changes> m_updates;
      bool m_is_source_transaction;
      boost::signals2::scoped_connection m_connection;

      void validate_columns(const std::vector<int>& columns) const;
      void index_rows(int first);
      void touch(const std::shared_ptr<Group>& group, int column);
      std::unique_ptr<TableAggregation> make_aggregation(
        const Group& group, int column) const;
      void insert(int source, int shared_depth);
      void remove_row(int row);
      void move_row(int source, int destination);
      void erase(int row, int shared_depth, const Changes& changes);
      void track(const Operation& operation);
      void flush_updates(const Operation* shift);
      void refresh_aggregates();
      void move_leaf(int source);
      void apply(const Operation& operation);
      void on_operation(const Operation& operation);
  };
}

#endif
