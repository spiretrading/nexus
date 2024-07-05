#ifndef SPIRE_ORDER_TASK_ARGUMENTS_LIST_TO_TABLE_MODEL_HPP
#define SPIRE_ORDER_TASK_ARGUMENTS_LIST_TO_TABLE_MODEL_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /** Enumerates the columns of the OrderTaskArgumentsListToTableModel. */
  enum class OrderTaskColumns {

    /** The name column. */
    NAME,

    /** The region column. */
    REGION,

    /** The destination column. */
    DESTINATION,

    /** The order type column. */
    ORDER_TYPE,

    /** The side column. */
    SIDE,

    /** The quantity column. */
    QUANTITY,

    /** The time in force column. */
    TIME_IN_FORCE,

    /** The additional tags column. */
    TAGS,

    /** The key binding column. */
    KEY
  };

  /** Provides a TableModel view over a an OrderTaskArgumentsListModel. */
  class OrderTaskArgumentsListToTableModel : public TableModel {
    public:

      /**
       * Constructs an OrderTaskArgumentsListToTableModel for a given
       * OrderTaskArgumentsListModel.
       * @param order_task_arguments The list of OrderTaskArguments to view as a
       *        table.
       */
      explicit OrderTaskArgumentsListToTableModel(
        std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State
        set(int row, int column, const std::any& value) override;

      QValidator::State remove(int row) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<OrderTaskArgumentsListModel> m_order_task_arguments;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_connection;

      void on_operation(
        const OrderTaskArgumentsListModel::Operation& operation);
  };
}

#endif
