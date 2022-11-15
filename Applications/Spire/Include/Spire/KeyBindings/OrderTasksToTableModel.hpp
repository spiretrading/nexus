#ifndef SPIRE_ORDER_TASKS_TO_TABLE_MODEL_HPP
#define SPIRE_ORDER_TASKS_TO_TABLE_MODEL_HPP
#include "Spire/KeyBindings/OrderTask.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements an OrderTasksToTableModel that converts a list of order tasks to
   * a TableModel. 
   */
  class OrderTasksToTableModel : public TableModel {
    public:

      /** Lists out the columns of the table. */
      enum class Column {

        /** The name field. */
        NAME,

        /** The region field. */
        REGION,

        /** The destination field. */
        DESTINATION,

        /** The order type field. */
        ORDER_TYPE,

        /** The side field. */
        SIDE,

        /** The quantity field. */
        QUANTITY,

        /** The time in force field. */
        TIME_IN_FORCE,

        /** The key field. */
        KEY
      };

      /** The number of columns in this table. */
      static const auto COLUMN_SIZE = 8;

      /**
       * Constructs an OrderTasksToTableModel.
       * @param source A list of OrderTask.
       */
      explicit OrderTasksToTableModel(
        std::shared_ptr<ListModel<OrderTask>> source);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State set(
        int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ListModel<OrderTask>> m_source;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      AnyRef extract_field(const OrderTask& order_task, Column column) const;
      void on_operation(const ListModel<OrderTask>::Operation& operation);
  };
}

#endif
