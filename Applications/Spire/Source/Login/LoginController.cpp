#include "Spire/Login/LoginController.hpp"
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/Login/LoginWindow.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

LoginController::LoginController(std::vector<ServerEntry> servers,
  ServiceClientsFactory service_clients_factory)
  : m_servers(std::move(servers)),
    m_service_clients_factory(std::move(service_clients_factory)),
    m_login_window(nullptr) {}

LoginController::~LoginController() = default;

ServiceClientsBox LoginController::get_service_clients() {
  return *m_service_clients;
}

void LoginController::open() {
  m_login_window = new LoginWindow(SPIRE_VERSION);
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
  m_login_promise = QtPromise([=] {
    return m_service_clients_factory(username, password,
      m_servers.front().m_address);
  }, LaunchPolicy::ASYNC).then(
    [=] (auto&& result) { on_login_promise(std::move(result)); });
}

void LoginController::on_cancel() {
  m_login_promise.disconnect();
  m_login_window->set_state(LoginWindow::State::NONE);
}

void LoginController::on_login_promise(
    Expect<ServiceClientsBox> service_clients) {
  try {
    m_service_clients.emplace(std::move(service_clients.Get()));
    m_login_window->close();
    delete_later(m_login_window);
    auto definitions = Definitions(
      m_service_clients->GetDefinitionsClient().LoadCountryDatabase(),
      m_service_clients->GetDefinitionsClient().LoadMarketDatabase(),
      m_service_clients->GetDefinitionsClient().LoadTimeZoneDatabase());
    m_service_clients->GetTimeClient().GetTime();
    m_logged_in_signal(definitions);
  } catch(const AuthenticationException&) {
    m_login_window->set_state(LoginWindow::State::INCORRECT_CREDENTIALS);
  } catch(const std::exception&) {
    m_login_window->set_state(LoginWindow::State::SERVER_UNAVAILABLE);
  }
}
