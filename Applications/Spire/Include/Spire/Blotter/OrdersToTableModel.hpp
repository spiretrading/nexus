#ifndef SPIRE_ORDERS_TO_TABLE_MODEL_HPP
#define SPIRE_ORDERS_TO_TABLE_MODEL_HPP
#include <unordered_map>
#include <vector>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/QtTaskQueue.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableModelTransactionLog.hpp"

namespace Spire {

  /** Flattens out the fields of a list of orders into a TableModel. */
  class OrdersToTableModel : public TableModel {
    public:

      /** Lists out the columns of the table. */
      enum class Column {

        /** The timestamp field. */
        TIME,

        /** The order ID field. */
        ID,

        /** The order's current status. */
        ORDER_STATUS,

        /** The order's security field. */
        SECURITY,

        /** The order's currency field. */
        CURRENCY,

        /** The order type field. */
        ORDER_TYPE,

        /** The order's side field. */
        SIDE,

        /** The order's destination field. */
        DESTINATION,

        /** The order's quantity field. */
        QUANTITY,

        /** The order's price field. */
        PRICE,

        /** The order's time in force field. */
        TIME_IN_FORCE
      };

      /** The number of columns in this table. */
      static const auto COLUMN_SIZE = 11;

      /**
       * Constructs an OrdersTableModel populated by a list of orders.
       * @param orders The list of orders used to populate this table.
       */
      explicit OrdersToTableModel(std::shared_ptr<OrderListModel> orders);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      struct OrderStatusEntry {
        Nexus::OrderStatus m_status;
        std::vector<int> m_indexes;
      };
      QtTaskQueue m_tasks;
      std::shared_ptr<OrderListModel> m_orders;
      std::unordered_map<const Nexus::OrderExecutionService::Order*,
        OrderStatusEntry> m_status_entries;
      TableModelTransactionLog m_transaction;
      boost::signals2::scoped_connection m_connection;

      AnyRef extract_field(
        const Nexus::OrderExecutionService::Order& order, Column column) const;
      void add(const Nexus::OrderExecutionService::Order& order, int index);
      void reindex(OrderStatusEntry& entry,
        const Nexus::OrderExecutionService::Order& order);
      void on_execution_report(const Nexus::OrderExecutionService::Order& order,
        const Nexus::OrderExecutionService::ExecutionReport& report);
      void on_operation(const OrderListModel::Operation& operation);
  };
}

#endif
