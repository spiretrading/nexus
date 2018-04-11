#include "spire/time_and_sales/time_and_sales_controller.hpp"
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

time_and_sales_controller::time_and_sales_controller(
    VirtualServiceClients& service_clients)
    : m_service_clients(&service_clients) {}

time_and_sales_controller::~time_and_sales_controller() = default;

void time_and_sales_controller::open() {
  if(m_window != nullptr) {
    return;
  }
  m_window = std::make_unique<time_and_sales_window>(
    time_and_sales_properties());
  m_window->connect_closed_signal([=] { on_closed(); });
  m_window->show();
}

connection time_and_sales_controller::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void time_and_sales_controller::on_change_security(const Security& s) {
  auto model = std::make_shared<empty_time_and_sales_model>(s);
  m_window->set_model(model);
}

void time_and_sales_controller::on_closed() {
  m_closed_signal();
}
