#include "spire/login/login_controller.hpp"
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/login/login_window.hpp"
#include "spire/spire/qt_promise.hpp"
#include "spire/version.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

LoginController::LoginController(std::vector<ServerEntry> servers,
    ServiceClientsFactory service_clients_factory)
    : m_servers(std::move(servers)),
      m_service_clients_factory(std::move(service_clients_factory)) {}

LoginController::~LoginController() = default;

std::unique_ptr<VirtualServiceClients>& LoginController::get_service_clients() {
  return m_service_clients;
}

void LoginController::open() {
  m_login_window = std::make_unique<LoginWindow>(SPIRE_VERSION);
  m_login_window->connect_login_signal(
    [=] (const auto& p1, const auto& p2) { on_login(p1, p2); });
  m_login_window->connect_cancel_signal([=] () { on_cancel(); });
  m_login_window->show();
}

connection LoginController::connect_logged_in_signal(
    const LoggedInSignal::slot_type& slot) const {
  return m_logged_in_signal.connect(slot);
}

void LoginController::on_login(const std::string& username,
    const std::string& password) {
  auto service_clients = m_service_clients_factory(username, password,
    m_servers.front().m_address);
  m_login_promise = make_qt_promise(
    [=, service_clients = std::move(service_clients)] () mutable {
      service_clients->Open();
      return std::move(service_clients);
    });
  m_login_promise.then(
    [=] (auto&& result) { on_login_promise(std::move(result)); });
}

void LoginController::on_cancel() {
  m_login_promise.disconnect();
  m_login_window->set_state(LoginWindow::State::NONE);
}

void LoginController::on_login_promise(
    Expect<std::unique_ptr<VirtualServiceClients>> service_clients) {
  try {
    m_service_clients = std::move(service_clients.Get());
    m_login_window->close();
    m_login_window.reset();
    m_logged_in_signal();
  } catch(const AuthenticationException&) {
    m_login_window->set_state(LoginWindow::State::INCORRECT_CREDENTIALS);
  } catch(const std::exception&) {
    m_login_window->set_state(LoginWindow::State::SERVER_UNAVAILABLE);
  }
}
