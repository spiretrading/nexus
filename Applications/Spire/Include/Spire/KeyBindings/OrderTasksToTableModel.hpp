#ifndef SPIRE_ORDER_TASKS_TO_TABLE_MODEL_HPP
#define SPIRE_ORDER_TASKS_TO_TABLE_MODEL_HPP
#include <QKeySequence>
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /** The order task with a keybinding. */
  struct OrderTask {

    /** The name of the order task. */
    QString m_name;

    /** The region this order task is avaialbe on. */
    Nexus::Region m_region;

    /** The destination this order task is avaialbe on. */
    Nexus::Destination m_destination;

    /** The order type. */
    Nexus::OrderType m_order_type;

    /** A bid or an ask for the order task. */
    Nexus::Side m_side;

    /** The quantity of the order task. */
    boost::optional<Nexus::Quantity> m_quantity;

    /** The expire of the order task. */
    Nexus::TimeInForce m_time_in_force;

    /** The keybinding of the order task. */
    QKeySequence m_key;
  };

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
