#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

NoneTimeAndSalesModel::NoneTimeAndSalesModel(Security security)
  : m_security(std::move(security)) {}


const Security& NoneTimeAndSalesModel::get_security() const {
  return m_security;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    NoneTimeAndSalesModel::query_until(Queries::Sequence sequence,
      int max_count) {
  return QtPromise([] {
    return std::vector<TimeAndSalesModel::Entry>();
  });
}

connection NoneTimeAndSalesModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return {};
}
