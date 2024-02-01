#ifndef SPIRE_SORTED_TABLE_MODEL_HPP
#define SPIRE_SORTED_TABLE_MODEL_HPP
#include <functional>
#include <vector>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"
#include "Spire/Spire/TranslatedTableModel.hpp"

namespace Spire {

  /**
   * Implements a TableModel that provides a view over a source model by sorting
   * a column in ascending order, descending order or without any order.
   */
  class SortedTableModel : public TableModel {
    public:

      /**
       * Binary function that accepts two elements and returns whether the first
       * argument is less than the second.
       * @return Returns <code>true</code> iff the first argument is less than
       *         the second.
       */
      using Comparator = std::function<bool (const AnyRef&, const AnyRef&)>;

      /** Specifies a sort order. */
      enum class Ordering {

        /** The column is unsorted. */
        NONE,

        /** The column is sorted from least to greatest. */
        ASCENDING,

        /** The column is sorted from greatest to least. */
        DESCENDING
      };

      /** Specifies the ordering applied to a specific column. */
      struct ColumnOrder {

        /** The index of the column. */
        int m_index;

        /** The column's sort order. */
        Ordering m_order;
      };

      /** 
       * Constructs a SortedTableModel from a TableModel.
       * @param source The model.
       */
      explicit SortedTableModel(std::shared_ptr<TableModel> source);

      /**
       * Constructs a SortedTableModel from a TableModel and a ColumnOrder. It
       * supports sorting by multiple columns. A default comparator will be used
       * to sort elements based on the columns and the sort orders in the order.
       * @param source The model.
       * @param order The multiple-column sorting order which comtains multiple
       *              columns and sort orders. The first element in the order
       *              is considered as the primary sorting key and the rest is
       *              the secondary sorting key.
       */
      SortedTableModel(std::shared_ptr<TableModel> source,
        std::vector<ColumnOrder> order);

      /**
       * Constructs a SortedTableModel from a TableModel and a comparator.
       * @param source The model.
       * @param compartor A comparison function.
       */
      SortedTableModel(std::shared_ptr<TableModel> source,
        Comparator comparator);

      /**
       * Constructs a SortedTableModel from a TableModel, a ColumnOrder and
       * a comparator. It supports sorting by multiple columns.
       * @param source The model.
       * @param order The multiple-column sorting order which comtains multiple
       *               columns and sort orders. The first element in the order
       *               is considered as the primary sorting key and the rest is
       *               the secondary sorting key.
       * @param compartor A comparison function.
       */
      SortedTableModel(std::shared_ptr<TableModel> source,
        std::vector<ColumnOrder> order, Comparator comparator);

      /** Returns the comparator used to rank rows. */
      const Comparator& get_comparator() const;

      /** Returns the multiple-column sort order in the model. */
      const std::vector<ColumnOrder>& get_column_order() const;

      /**
       * Sets the column sort order.
       * @param order The column sort order.
       */
      void set_column_order(const std::vector<ColumnOrder>& order);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State set(
        int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      TranslatedTableModel m_translation;
      std::vector<ColumnOrder> m_order;
      Comparator m_comparator;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      bool row_comparator(int lhs, int rhs) const;
      void sort();
      int find_sorted_index(int row, int size) const;
      void on_operation(const Operation& operation);
  };

  /**
   * Sets the order to be the primary sorting key in the multiple-column sorting
   * order.
   * @param order The order which will be the primary sorting key.
   * @param column_order The multiple-column sorting order.
   */
  void adjust(SortedTableModel::ColumnOrder order,
    std::vector<SortedTableModel::ColumnOrder>& column_order);

  /**
   * The cycles a ColumnOrder's sort order from none, to ascending, to
   * descending.
   */
  SortedTableModel::ColumnOrder cycle(SortedTableModel::ColumnOrder order);
}

#endif
