#include "Spire/Login/LoginController.hpp"
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Login/LoginException.hpp"
#include "Spire/Login/LoginWindow.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

LoginController::LoginController(
  std::string version, std::vector<ServerEntry> servers,
  ServiceClientsFactory service_clients_factory)
  : m_version(std::move(version)),
    m_servers(std::move(servers)),
    m_service_clients_factory(std::move(service_clients_factory)),
    m_login_window(nullptr) {}

void LoginController::open() {
  m_login_window = new LoginWindow(m_version);
  m_login_window->connect_login_signal(
    std::bind_front(&LoginController::on_login, this));
  m_login_window->connect_cancel_signal(
    std::bind_front(&LoginController::on_cancel, this));
  m_login_window->show();
}

connection LoginController::connect_logged_in_signal(
    const LoggedInSignal::slot_type& slot) const {
  return m_logged_in_signal.connect(slot);
}

void LoginController::on_login(
    const std::string& username, const std::string& password) {
  m_login_promise = QtPromise([=] {
    return m_service_clients_factory(
      username, password, m_servers.front().m_address);
  }, LaunchPolicy::ASYNC).then(
    std::bind_front(&LoginController::on_login_promise, this));
}

void LoginController::on_cancel() {
  m_login_promise.disconnect();
  m_login_window->set_state(LoginWindow::State::NONE);
}

void LoginController::on_login_promise(
    Expect<ServiceClientsBox> service_clients) {
  if(service_clients.IsException()) {
    try {
      std::rethrow_exception(service_clients.GetException());
    } catch(const AuthenticationException&) {
      m_login_window->set_error(QObject::tr("Incorrect username or password."));
    } catch(const LoginException& e) {
      m_login_window->set_error(QObject::tr(e.what()));
    } catch(const std::exception&) {
      m_login_window->set_error(QObject::tr("Server unavailable."));
    }
    return;
  }
  m_login_window->close();
  delete_later(m_login_window);
  m_logged_in_signal(std::move(service_clients.Get()));
}
