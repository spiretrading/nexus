#ifndef SPIRE_ORDERS_TO_EXECUTION_REPORT_LIST_MODEL_HPP
#define SPIRE_ORDERS_TO_EXECUTION_REPORT_LIST_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/BlotterModel.hpp"

namespace Spire {

  /**
   * Implements a ListModel listing out all execution reports published by a
   * specified list of orders.
   */
  class OrdersToExecutionReportListModel : public ExecutionReportListModel {
    public:

      /**
       * Constructs a DerivedOrderListModel.
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
  };
}

#endif
