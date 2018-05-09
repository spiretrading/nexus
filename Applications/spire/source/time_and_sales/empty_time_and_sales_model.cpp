#include "spire/time_and_sales/empty_time_and_sales_model.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

empty_time_and_sales_model::empty_time_and_sales_model(Security s)
    : m_security(std::move(s)) {}

const Nexus::Security& empty_time_and_sales_model::get_security() const {
  return m_security;
}

Quantity empty_time_and_sales_model::get_volume() const {
  return 0;
}

qt_promise<std::vector<time_and_sales_model::entry>>
    empty_time_and_sales_model::load_snapshot(Beam::Queries::Sequence last,
    int count) {
  return make_qt_promise([] {
    return std::vector<time_and_sales_model::entry>();
  });
}

connection empty_time_and_sales_model::connect_time_and_sale_signal(
    const time_and_sale_signal::slot_type& slot) const {
  return {};
}

connection empty_time_and_sales_model::connect_volume_signal(
    const volume_signal::slot_type& slot) const {
  return {};
}
