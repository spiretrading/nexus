#include "spire/toolbar/toolbar_controller.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/time_and_sales/time_and_sales_controller.hpp"
#include "spire/toolbar/toolbar_window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

struct ToolbarController::BaseController {
  virtual void open() = 0;
};

template<typename T>
struct ToolbarController::Controller : ToolbarController::BaseController {
  using Type = T;
  Type m_controller;

  template<typename... Args>
  Controller(Args&&... args)
      : m_controller(std::forward<Args>(args)...) {}

  void open() override {
    m_controller.open();
  }
};

ToolbarController::ToolbarController(VirtualServiceClients& service_clients)
    : m_service_clients(&service_clients) {}

ToolbarController::~ToolbarController() = default;

void ToolbarController::open() {
  if(m_toolbar_window != nullptr) {
    return;
  }
  m_toolbar_window = std::make_unique<ToolbarWindow>(m_model,
    m_service_clients->GetServiceLocatorClient().GetAccount());
  m_toolbar_window->connect_open_signal(
    [=] (auto w) { on_open_window(w); });
  m_toolbar_window->connect_closed_signal([=] { on_closed(); });
  m_toolbar_window->show();
}

connection ToolbarController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void ToolbarController::on_open_window(RecentlyClosedModel::Type w) {
  if(w == RecentlyClosedModel::Type::TIME_AND_SALE) {
    auto c = std::make_unique<Controller<TimeAndSalesController>>(
      *m_service_clients);
    c->open();
    m_controllers.push_back(std::move(c));
  }
}

void ToolbarController::on_closed() {
  m_closed_signal();
}
