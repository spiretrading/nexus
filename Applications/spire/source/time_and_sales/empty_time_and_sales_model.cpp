#include "spire/time_and_sales/empty_time_and_sales_model.hpp"

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

connection empty_time_and_sales_model::connect_time_and_sale_signal(
    const time_and_sale_signal::slot_type& slot) const {
  return {};
}

connection empty_time_and_sales_model::connect_volume_signal(
    const volume_signal::slot_type& slot) const {
  return {};
}
