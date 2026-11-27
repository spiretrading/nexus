#ifndef SPIRE_GROUPED_TABLE_MODEL_HPP
#define SPIRE_GROUPED_TABLE_MODEL_HPP
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <unordered_map>
#include "Spire/Spire/SortedTableModel.hpp"
#include "Spire/Spire/TransactionLogModel.hpp"
#include "Spire/Ui/TableAggregator.hpp"

namespace Spire {

  /**
   * Groups source rows by value and displays aggregates in each heading.
   * Values without a supported equality comparison form separate groups.
   */
  class GroupedTableModel : public TransactionLogModel<TableModel> {
    public:

      /** Creates an independent aggregator for one column in one group. */
      using AggregatorFactory =
        std::function<std::unique_ptr<TableAggregator> ()>;

      /**
       * Tests whether two aggregate snapshots are interchangeable.
       * @param previous The last published value of the aggregate cell.
       * @param current The newly computed value of the same cell.
       * @return True if the cell update can be skipped.
       */
      using AggregateEquality =
        std::function<bool (const AnyRef& previous, const AnyRef& current)>;

      /**
       * Signals the new grouping columns after regrouping.
       * @param columns The source column indices from outermost to innermost
       *        group, or an empty vector when the table is ungrouped.
       */
      using GroupingSignal = Signal<void (const std::vector<int>& columns)>;

      /**
       * Sibling rows are compared by group key, aggregate, or leaf value.
       * Row indices refer to the grouped model and the column to the source.
       */
      using Comparator = SortedTableModel::Comparator;

      using ValueComparator = SortedTableModel::ValueComparator;

      /**
       * Constructs an ungrouped model using the default column aggregators.
       * @param source The table containing the leaves.
       */
      explicit GroupedTableModel(std::shared_ptr<TableModel> source);

      /**
       * Constructs an ungrouped model with a custom comparator.
       * @param source The table whose rows form the leaves.
       * @param comparator Orders sibling values, optionally taking their
       *        grouped row indices and source column index.
       */
      template<typename C> requires
        (std::convertible_to<C, SortedTableModel::Comparator> ||
          std::convertible_to<C, SortedTableModel::ValueComparator>)
      GroupedTableModel(std::shared_ptr<TableModel> source, C comparator);

      /**
       * Constructs an ungrouped model with custom column aggregators.
       * @param source The table containing the leaves.
       * @param aggregators Factories indexed by source column. Omitted or empty
       *        entries use the default aggregator.
       */
      GroupedTableModel(std::shared_ptr<TableModel> source,
        std::vector<AggregatorFactory> aggregators);

      /**
       * Constructs an ungrouped model with aggregators and a value comparator.
       * @param source The table whose rows form the leaves.
       * @param aggregators Factories indexed by source column. Omitted or empty
       *        entries use the default aggregator.
       * @param comparator Returns true if the first sibling value sorts
       *        before the second.
       */
      GroupedTableModel(std::shared_ptr<TableModel> source,
        std::vector<AggregatorFactory> aggregators, ValueComparator comparator);

      /**
       * Constructs an ungrouped model with aggregators and a row comparator.
       * @param source The table whose rows form the leaves.
       * @param aggregators Factories indexed by source column. Omitted or empty
       *        entries use the default aggregator.
       * @param comparator Orders sibling values given their grouped row indices
       *        and source column index. An empty comparator uses the default.
       */
      GroupedTableModel(std::shared_ptr<TableModel> source,
        std::vector<AggregatorFactory> aggregators, Comparator comparator);

      /**
       * Constructs an ungrouped model with table-wide aggregate equality.
       * @param source The table whose rows form the leaves.
       * @param aggregators Factories indexed by source column. Omitted or empty
       *        entries use the default aggregator.
       * @param comparator Orders sibling values using their grouped row indices
       *        and source column index, or empty to use the default ordering.
       * @param is_equal Compares previous and current aggregate values across
       *        all columns, or empty to use Spire::is_equal.
       */
      GroupedTableModel(std::shared_ptr<TableModel> source,
        std::vector<AggregatorFactory> aggregators, Comparator comparator,
        AggregateEquality is_equal);

      ~GroupedTableModel() override;

      /** Returns the table containing the leaves. */
      const std::shared_ptr<TableModel>& get_source() const;

      /** Returns the comparator used to order siblings. */
      const Comparator& get_comparator() const;

      /** Returns the grouping columns, from outermost to innermost. */
      const std::vector<int>& get_grouping() const;

      /**
       * Sets the grouping columns.
       * @param columns Distinct source column indices from outermost to
       *        innermost group, or an empty vector to ungroup the table.
       */
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

      /** Returns the number of rows beneath a heading, or zero for a leaf. */
      int get_descendant_count(int row) const;

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
      struct PendingRows;
      using Changes = std::map<int, std::any>;
      using GroupIndex = std::unordered_multimap<std::size_t, Group*>;
      mutable GroupingSignal m_grouping_signal;
      std::shared_ptr<TableModel> m_source;
      std::optional<int> m_column_count;
      bool m_has_matching_columns;
      std::vector<int> m_columns;
      std::vector<AggregatorFactory> m_aggregators;
      Comparator m_comparator;
      AggregateEquality m_is_equal;
      std::vector<SortedTableModel::ColumnOrder> m_order;
      std::set<std::shared_ptr<Group>> m_unsorted;
      std::size_t m_next_group;
      GroupIndex m_groups;
      std::vector<Row> m_rows;
      PendingRows* m_pending_rows;
      std::vector<int> m_indices;
      std::vector<std::pair<std::shared_ptr<Group>, int>> m_dirty;
      std::map<int, Changes> m_updates;
      bool m_is_source_transaction;
      boost::signals2::scoped_connection m_connection;

      static Group* find_group(const GroupIndex& groups, const AnyRef& key,
        const Group* parent, std::optional<std::size_t> hash);
      int get_source_row_size() const;
      void set_column_count(int count);
      void validate_columns(const std::vector<int>& columns) const;
      bool is_identity() const;
      int resolve_index(int index) const;
      const Row& get_row(int index) const;
      void materialize_rows();
      void group_leaves(std::vector<int> columns);
      std::vector<Row> make_grouped_rows(const std::vector<int>& columns);
      void append_groups(std::vector<Row>& rows, GroupIndex& groups,
        const std::vector<int>& sources, const std::vector<int>& columns,
        const std::shared_ptr<Group>& parent, int depth);
      void restore_source_order();
      void index_rows(int first, int end);
      bool is_sorted(int column) const;
      AnyRef sort_value(const Row& row, int column) const;
      void sort_siblings(const std::shared_ptr<Group>& parent);
      void sort_all();
      void sort_pending();
      void touch(const std::shared_ptr<Group>& group, int column);
      std::unique_ptr<TableAggregator> make_aggregator(int column) const;
      std::shared_ptr<Group> make_group(const AnyRef& key, QString name,
        const std::shared_ptr<Group>& parent, int depth, int heading_column);
      void initialize_aggregates(Group& group, std::span<const int> sources);
      void accumulate(const std::shared_ptr<Group>& group, int source);
      void insert(int source, std::shared_ptr<Group> parent);
      void remove_row(int row);
      void move_row(int source, int destination);
      void remove_from_groups(int source, std::shared_ptr<Group> group,
        int shared_depth, const Changes& changes);
      void track(const Operation& operation);
      void flush_updates(const Operation* shift);
      void refresh_aggregates();
      void move_leaf(int source);
      void apply(const Operation& operation);
      void on_operation(const Operation& operation);
  };

  template<typename C> requires
    (std::convertible_to<C, SortedTableModel::Comparator> ||
      std::convertible_to<C, SortedTableModel::ValueComparator>)
  GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
    C comparator)
    : GroupedTableModel(std::move(source), {}, std::move(comparator)) {}
}

#endif
