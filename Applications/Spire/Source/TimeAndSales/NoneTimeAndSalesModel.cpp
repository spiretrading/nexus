#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"

using namespace boost::signals2;
using namespace Spire;

QtPromise<void> NoneTimeAndSalesModel::load_older(int max_count) {
  return QtPromise<void>();
}

int NoneTimeAndSalesModel::get_size() const {
  return 0;
}

const NoneTimeAndSalesModel::Type& NoneTimeAndSalesModel::get(int index) const {
  throw std::out_of_range("The index is out of range.");
}

connection NoneTimeAndSalesModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return {};
}

void NoneTimeAndSalesModel::transact(
    const std::function<void ()>& transaction) {
  transaction();
}
