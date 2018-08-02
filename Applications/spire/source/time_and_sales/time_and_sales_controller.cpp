#include "spire/time_and_sales/time_and_sales_controller.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeAndSalesController::TimeAndSalesController(
    VirtualServiceClients& service_clients)
    : m_service_clients(&service_clients),
      m_input_model(std::make_unique<LocalSecurityInputModel>()) {}

TimeAndSalesController::~TimeAndSalesController() = default;

void TimeAndSalesController::open() {
  if(m_window != nullptr) {
    return;
  }
  m_window = std::make_unique<TimeAndSalesWindow>(
    TimeAndSalesProperties(), *m_input_model);
  m_window->connect_closed_signal([=] { on_closed(); });
  m_window->show();
}

connection TimeAndSalesController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void TimeAndSalesController::on_change_security(const Security& s) {
  auto model = std::make_shared<EmptyTimeAndSalesModel>(s);
  m_window->set_model(model);
}

void TimeAndSalesController::on_closed() {
  m_closed_signal();
}
