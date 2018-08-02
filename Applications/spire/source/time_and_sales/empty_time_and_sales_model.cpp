#include "spire/time_and_sales/empty_time_and_sales_model.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

EmptyTimeAndSalesModel::EmptyTimeAndSalesModel(Security s)
    : m_security(std::move(s)) {}

const Nexus::Security& EmptyTimeAndSalesModel::get_security() const {
  return m_security;
}

Quantity EmptyTimeAndSalesModel::get_volume() const {
  return 0;
}

QtPromise<std::vector<TimeAndSalesModel::Entry>>
    EmptyTimeAndSalesModel::load_snapshot(Beam::Queries::Sequence last,
    int count) {
  return make_qt_promise([] {
    return std::vector<TimeAndSalesModel::Entry>();
  });
}

connection EmptyTimeAndSalesModel::connect_time_and_sale_signal(
    const TimeAndSaleSignal::slot_type& slot) const {
  return {};
}

connection EmptyTimeAndSalesModel::connect_volume_signal(
    const VolumeSignal::slot_type& slot) const {
  return {};
}
