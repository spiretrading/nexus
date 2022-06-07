#include "Spire/Blotter/OrdersToExecutionReportListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

OrdersToExecutionReportListModel::OrdersToExecutionReportListModel(
  std::shared_ptr<OrderListModel> orders) {}

int OrdersToExecutionReportListModel::get_size() const {
  throw std::runtime_error("Not implemented.");
}

const OrdersToExecutionReportListModel::Type&
    OrdersToExecutionReportListModel::get(int index) const {
  throw std::runtime_error("Not implemented.");
}

connection OrdersToExecutionReportListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  throw std::runtime_error("Not implemented.");
}

void OrdersToExecutionReportListModel::transact(
    const std::function<void ()>& transaction) {
  transaction();
}
