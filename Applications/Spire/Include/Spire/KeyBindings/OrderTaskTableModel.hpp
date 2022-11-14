#ifndef SPIRE_ORDER_TASK_TABLE_MODEL_HPP
#define SPIRE_ORDER_TASK_TABLE_MODEL_HPP
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

  /** The field index of the grab handle. */
  static const auto GRAB_HANDLE_INDEX = 0;

  /** The field index of the name. */
  static const auto NAME_INDEX = 1;

  /** The field index of the region. */
  static const auto REGION_INDEX = 2;

  /** The field index of the destination. */
  static const auto DESTINATION_INDEX = 3;

  /** The field index of the order type. */
  static const auto ORDER_TYPE_INDEX = 4;

  /** The field index of the side. */
  static const auto SIDE_INDEX = 5;

  /** The field index of the quantity. */
  static const auto QUANTITY_INDEX = 6;

  /** The field index of the time in force. */
  static const auto TIME_IN_FORCE_INDEX = 7;

  /** The field index of the key. */
  static const auto KEY_INDEX = 8;

  /**
   * Implements a OrderTaskTableModel that converts a list of OrderTask to
   * the table model used by the OrderTasksPage
   */
  class OrderTaskTableModel : public TableModel {
    public:

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

        /** The key binding of the order task. */
        QKeySequence m_key_sequence;
      };

      /**
       * Constructs a OrderTasksTableModel.
       * @param source The model.
       */
      explicit OrderTaskTableModel(
        std::shared_ptr<ListModel<OrderTask>> source);

      /**
       * Appends an order task.
       * @param order_task The order task to append to this model.
       */
      void push(const OrderTask& order_task);

      /**
       * Moves a row.
       * @param source The index of the row to move.
       * @param destination The index to move the row to.
       * @throws <code>std::out_of_range</code> - The source or destination are
       *         not within this table's range.
       */
      void move(int source, int destination);

      /**
       * Removes a row from the table.
       * @param index - The index of the row to remove.
       * @throws <code>std::out_of_range</code> - The index is not within this
       *         table's range.
       */
      void remove(int index);

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

      void on_operation(const AnyListModel::Operation& operation);
  };
}

#endif
