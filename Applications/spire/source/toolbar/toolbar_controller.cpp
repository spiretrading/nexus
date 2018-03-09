#include "spire/toolbar/toolbar_controller.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/toolbar/toolbar_window.hpp"

using namespace Nexus;
using namespace spire;

toolbar_controller::toolbar_controller(VirtualServiceClients& service_clients)
    : m_service_clients(&service_clients) {}

toolbar_controller::~toolbar_controller() = default;

void toolbar_controller::open() {
  m_toolbar_window = std::make_unique<toolbar_window>();
  m_toolbar_window->show();
}
