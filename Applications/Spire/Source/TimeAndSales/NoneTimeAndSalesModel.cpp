#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    NoneTimeAndSalesModel::query_until(Sequence sequence, int max_count) {
  return std::vector<Entry>();
}

connection NoneTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return {};
}
