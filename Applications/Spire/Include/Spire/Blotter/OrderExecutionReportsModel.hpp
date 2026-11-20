#ifndef SPIRE_ORDER_EXECUTION_REPORTS_MODEL_HPP
#define SPIRE_ORDER_EXECUTION_REPORTS_MODEL_HPP
#include <unordered_map>
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Blotter/ExecutionReportsModel.hpp"
#include "Spire/Blotter/OrdersModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Collects execution reports, deduplicated by order ID and sequence. */
  class OrderExecutionReportsModel : public ExecutionReportsModel {
    public:

      /**
       * Constructs an OrderExecutionReportsModel.
       * @param orders The orders whose execution reports are collected.
       */
      explicit OrderExecutionReportsModel(std::shared_ptr<OrdersModel> orders);

      ~OrderExecutionReportsModel() override;

      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
      void start_transaction() override;
      void end_transaction() override;
      using ExecutionReportsModel::remove;

    private:
      struct OrderEntry;
      std::shared_ptr<OrdersModel> m_orders;
      ArrayListModel<Type> m_reports;
      std::unordered_map<Nexus::OrderId, std::unordered_map<int, int>> m_counts;
      std::vector<std::shared_ptr<OrderEntry>> m_entries;
      QtTaskQueue m_events;
      boost::signals2::scoped_connection m_orders_connection;

      void add(int index, std::shared_ptr<Nexus::Order> order);
      void remove(const OrderEntry& entry);
      void on_report(OrderEntry& entry, const Nexus::ExecutionReport& report);
      void on_operation(const OrdersModel::Operation& operation);
  };
}

#endif
