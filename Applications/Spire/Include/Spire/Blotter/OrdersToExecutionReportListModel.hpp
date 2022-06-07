#ifndef SPIRE_ORDERS_TO_EXECUTION_REPORT_LIST_MODEL_HPP
#define SPIRE_ORDERS_TO_EXECUTION_REPORT_LIST_MODEL_HPP
#include <unordered_map>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/QtTaskQueue.hpp"

namespace Spire {

  /**
   * Implements a ListModel listing out all execution reports published by a
   * specified list of orders.
   */
  class OrdersToExecutionReportListModel : public ExecutionReportListModel {
    public:

      /**
       * Constructs an OrdersToExecutionReportListModel.
       * @param orders The list of orders producing the execution reports that
       *        this model is to publish.
       */
      explicit OrdersToExecutionReportListModel(
        std::shared_ptr<OrderListModel> orders);

      int get_size() const override;

      const Type& get(int index) const override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      QtTaskQueue m_queue;
      std::shared_ptr<OrderListModel> m_orders;
      std::shared_ptr<ArrayListModel<
        Nexus::OrderExecutionService::ExecutionReportEntry>> m_reports;
      std::unordered_map<const Nexus::OrderExecutionService::Order*, int>
        m_order_to_count;
      boost::signals2::scoped_connection m_connection;

      void add(const Nexus::OrderExecutionService::Order& order);
      void remove(const Nexus::OrderExecutionService::Order& order);
      void on_report(const Nexus::OrderExecutionService::Order& order,
        const Nexus::OrderExecutionService::ExecutionReport& report);
      void on_operation(const OrderListModel::Operation& operation);
  };
}

#endif
