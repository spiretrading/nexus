#include "spire/toolbar/toolbar_controller.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/toolbar/toolbar_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

toolbar_controller::toolbar_controller(VirtualServiceClients& service_clients)
    : m_service_clients(&service_clients) {}

toolbar_controller::~toolbar_controller() = default;

void toolbar_controller::open() {
  if(m_toolbar_window != nullptr) {
    return;
  }
  m_toolbar_window = std::make_unique<toolbar_window>();
  m_toolbar_window->connect_closed_signal([=] {on_closed();});
  m_toolbar_window->show();
}

connection toolbar_controller::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void toolbar_controller::on_closed() {
  m_closed_signal();
}
