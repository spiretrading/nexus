#ifndef SPIRE_SORTED_TABLE_MODEL_HPP
#define SPIRE_SORTED_TABLE_MODEL_HPP
#include <functional>
#include <vector>
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/TableModelTransactionLog.hpp"
#include "Spire/Ui/TranslatedTableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a TableModel that provides a view over a source model by
   * sorting a column in ascending order, descending order or without any order.
   */
  class SortedTableModel : public TableModel {
    public:

      /**
       * Binary function that accepts two elements, and returns a value
       * convertible to bool. The value returned indicates whether the element
       * passed as first argument is considered to be less than the second.
       * @return Returns true when the first argument is less than the second,
       *         otherwise returns false.
       */
      using Comparator = std::function<bool(const std::any&, const std::any&)>;

      /** Indicates the sorting order. */
      enum class Ordering {

        /** The unsorted order. */
        NONE,

        /** The ascending order. */
        ASCENDING,

        /** The descending order. */
        DESCENDING
      };

      /** Indicates a column will be sorted in a specific order. */
      struct ColumnOrder {

        /** Indicates a column will be sorted. */
        int m_index;

        /** Indicates a sort order. */
        Ordering m_order;

        /** The sort order cycles through unsorted, ascending and descending. */
        ColumnOrder cycle() const;
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
       *               columns and sort orders. The first element in the order
       *               is considered as the primary sorting key and the rest is
       *               the secondary sorting key.
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

      /** Returns the comparsion function in the model. */
      const Comparator& get_comparator() const;

      /** Returns the multiple-column sorting order in the model. */
      const std::vector<ColumnOrder>& get_column_order() const;

      /**
       * Sets the multiple-column sorting order.
       * @param order The multiple-column sorting order.
       */
      void set_column_order(const std::vector<ColumnOrder>& order);

      int get_row_size() const override;

      int get_column_size() const override;

      const std::any& at(int row, int column) const override;

      QValidator::State set(int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_source;
      std::vector<ColumnOrder> m_order;
      Comparator m_comparator;
      std::vector<int> m_mapping;
      std::vector<int> m_reverse_mapping;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      Comparator get_default_comparator();
      void sort();
      void translate(int direction, int row);
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
}

#endif
