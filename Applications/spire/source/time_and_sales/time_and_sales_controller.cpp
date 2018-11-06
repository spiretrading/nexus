#include "spire/time_and_sales/time_and_sales_controller.hpp"
#include "spire/time_and_sales/services_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_window.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeAndSalesController::TimeAndSalesController(
    Beam::Ref<SecurityInputModel> security_input_model,
    Beam::Ref<VirtualServiceClients> service_clients)
    : m_security_input_model(security_input_model.Get()),
      m_service_clients(service_clients.Get()) {}

TimeAndSalesController::~TimeAndSalesController() {
  close();
}

void TimeAndSalesController::open() {
  if(m_window != nullptr) {
    return;
  }
  m_window = std::make_unique<TimeAndSalesWindow>(TimeAndSalesProperties(),
    *m_security_input_model);
  m_window->connect_security_change_signal(
    [=] (const auto& security) { on_change_security(security); });
  m_window->connect_closed_signal([=] { on_closed(); });
  m_window->show();
}

void TimeAndSalesController::close() {
  if(m_window == nullptr) {
    return;
  }
  m_window.reset();
  m_closed_signal();
}

connection TimeAndSalesController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void TimeAndSalesController::on_change_security(const Security& security) {
  auto model = std::make_shared<ServicesTimeAndSalesModel>(security,
    Ref(*m_service_clients));
  m_window->set_model(model);
}

void TimeAndSalesController::on_closed() {
  close();
}
