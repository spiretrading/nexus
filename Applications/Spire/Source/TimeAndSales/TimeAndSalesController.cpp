#include "Spire/TimeAndSales/TimeAndSalesController.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/TimeAndSales/ServicesTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Ui/RecentColors.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeAndSalesController::TimeAndSalesController(Definitions definitions,
  Ref<SecurityInputModel> security_input_model,
  ServiceClientsBox service_clients)
  : m_definitions(std::move(definitions)),
    m_security_input_model(security_input_model.Get()),
    m_service_clients(std::move(service_clients)),
    m_window(nullptr) {}

TimeAndSalesController::~TimeAndSalesController() {
  close();
}

void TimeAndSalesController::open() {
  if(m_window != nullptr) {
    return;
  }
  m_window = new TimeAndSalesWindow(TimeAndSalesProperties(),
    Ref(*m_security_input_model));
  m_window->connect_change_security_signal(
    [=] (const auto& security) { on_change_security(security); });
  // TODO
  //m_window->connect_closed_signal([=] { on_closed(); });
  m_window->show();
}

void TimeAndSalesController::close() {
  if(m_window == nullptr) {
    return;
  }
  delete_later(m_window);
  m_closed_signal();
}

connection TimeAndSalesController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void TimeAndSalesController::on_change_security(const Security& security) {
  auto model = std::make_shared<ServicesTimeAndSalesModel>(security,
    m_definitions, m_service_clients);
  m_window->set_model(model);
}

void TimeAndSalesController::on_closed() {
  close();
}
