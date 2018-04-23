#include "spire/toolbar/toolbar_controller.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/time_and_sales/time_and_sales_controller.hpp"
#include "spire/toolbar/toolbar_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

struct toolbar_controller::base_controller {
  virtual void open() = 0;
};

template<typename T>
struct toolbar_controller::controller : toolbar_controller::base_controller {
  using type = T;
  type m_controller;

  template<typename... Args>
  controller(Args&&... args)
      : m_controller(std::forward<Args>(args)...) {}

  void open() override {
    m_controller.open();
  }
};

toolbar_controller::toolbar_controller(VirtualServiceClients& service_clients)
    : m_service_clients(&service_clients) {}

toolbar_controller::~toolbar_controller() = default;

void toolbar_controller::open() {
  if(m_toolbar_window != nullptr) {
    return;
  }
  m_toolbar_window = std::make_unique<toolbar_window>(m_model,
    m_service_clients->GetServiceLocatorClient().GetAccount());

  // GCC workaround
  m_toolbar_window->connect_open_signal(
    [=] (auto w) { this->on_open_window(w); });
  m_toolbar_window->connect_closed_signal([=] { on_closed(); });
  m_toolbar_window->show();
}

connection toolbar_controller::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void toolbar_controller::on_open_window(recently_closed_model::type w) {
  if(w == recently_closed_model::type::TIME_AND_SALE) {
    auto c = std::make_unique<controller<time_and_sales_controller>>(
      *m_service_clients);
    c->open();
    m_controllers.push_back(std::move(c));
  }
}

void toolbar_controller::on_closed() {
  m_closed_signal();
}
