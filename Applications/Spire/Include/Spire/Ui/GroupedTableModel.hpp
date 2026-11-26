#ifndef SPIRE_GROUPED_TABLE_MODEL_HPP
#define SPIRE_GROUPED_TABLE_MODEL_HPP
#include <map>
#include <optional>
#include <set>
#include "Spire/Spire/SortedTableModel.hpp"
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

      /** Sets the grouping columns. */
      void set_grouping(std::vector<int> columns);

      /** Sorts siblings by column priority. An empty order clears sorting. */
      void set_column_order(std::vector<SortedTableModel::ColumnOrder> order);

      /** Connects a slot to changes in the grouping columns. */
      boost::signals2::connection connect_grouping_signal(
        const GroupingSignal::slot_type& slot) const;

      /** Returns the parent heading's row, or -1 for a top-level row. */
      int get_parent(int row) const;

      /** Returns the name of a group heading, or an empty string for a leaf. */
      QString get_group_name(int row) const;

      /** Returns the number of leaves in a group, or zero for a leaf row. */
      int get_group_size(int row) const;

      /** Returns the grouped index of the first leaf in a group, or -1. */
      int get_first_leaf(int row) const;

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
      friend class GroupedTableViewBuilder;
      struct Group;
      struct Row;
      using Changes = std::map<int, std::any>;
      mutable GroupingSignal m_grouping_signal;
      std::shared_ptr<TableModel> m_source;
      std::optional<int> m_column_count;
      bool m_has_matching_columns;
      std::vector<int> m_columns;
      std::vector<TableAggregationFactory> m_aggregations;
      std::vector<SortedTableModel::ColumnOrder> m_order;
      std::set<std::shared_ptr<Group>> m_unsorted;
      std::size_t m_next_group;
      std::vector<Row> m_rows;
      std::vector<int> m_indices;
      std::vector<std::pair<std::shared_ptr<Group>, int>> m_dirty;
      std::map<int, Changes> m_updates;
      bool m_is_source_transaction;
      boost::signals2::scoped_connection m_connection;

      int get_source_row_size() const;
      void set_column_count(int count);
      void validate_columns(const std::vector<int>& columns) const;
      bool is_identity() const;
      void materialize_rows();
      void restore_source_order();
      void index_rows(int first, int end);
      bool is_sorted(int column) const;
      AnyRef sort_value(const Row& row, int column) const;
      void sort_siblings(const std::shared_ptr<Group>& parent);
      void sort_all();
      void sort_pending();
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
