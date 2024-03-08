#ifndef SPIRE_ORDER_TASK_ARGUMENTS_TO_TABLE_MODEL_HPP
#define SPIRE_ORDER_TASK_ARGUMENTS_TO_TABLE_MODEL_HPP
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /**
   * Implements an OrderTaskArgumentsToTableModel that converts a list of
   * OrderTaskArguments to a TableModel. 
   */
  class OrderTaskArgumentsToTableModel : public TableModel {
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

        /** The tag field. */
        TAG,

        /** The key field. */
        KEY
      };

      /** The number of columns in this table. */
      static const auto COLUMN_SIZE = 9;

      /**
       * Constructs an OrderTaskArgumentsToTableModel.
       * @param source A list of OrderTask.
       */
      explicit OrderTaskArgumentsToTableModel(
        std::shared_ptr<OrderTaskArgumentsListModel> source);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State set(
        int row, int column, const std::any& value) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<OrderTaskArgumentsListModel> m_source;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_source_connection;

      AnyRef extract_field(const OrderTaskArguments& arguments,
        Column column) const;
      void on_operation(
        const OrderTaskArgumentsListModel::Operation& operation);
  };
}

#endif
