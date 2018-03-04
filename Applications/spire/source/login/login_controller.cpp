#include "spire/login/login_controller.hpp"
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/login/login_window.hpp"
#include "spire/spire/qt_promise.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;
using namespace std;

login_controller::login_controller(
    service_clients_factory service_clients_factory)
    : m_service_clients_factory(std::move(service_clients_factory)) {}

login_controller::~login_controller() = default;

unique_ptr<VirtualServiceClients>& login_controller::get_service_clients() {
  return m_service_clients;
}

void login_controller::open() {
  m_login_window = std::make_unique<login_window>();
  m_login_window->connect_login_signal(
    [=] (auto&& p1, auto&& p2) {on_login(p1, p2);});
  m_login_window->connect_cancel_signal([=] () {on_cancel();});
  m_login_window->show();
}

connection login_controller::connect_logged_in_signal(
    const logged_in_signal::slot_type& slot) const {
  return m_logged_in_signal.connect(slot);
}

void login_controller::on_login(const string& username,
    const string& password) {
  auto service_clients = m_service_clients_factory(username, password);
  m_login_promise = qt_promise(
    [=, service_clients = std::move(service_clients)] () mutable {
      service_clients->Open();
      return std::move(service_clients);
    },
    [=] (auto&& result) {on_login_promise(std::move(result));});
}

void login_controller::on_cancel() {
  m_login_promise.disconnect();
  m_login_window->set_state(login_window::state::NONE);
}

void login_controller::on_login_promise(
    Expect<std::unique_ptr<VirtualServiceClients>> service_clients) {
  try {
    m_service_clients = std::move(service_clients.Get());
    m_login_window->close();
    m_login_window.reset();
    m_logged_in_signal();
  } catch(const AuthenticationException&) {
    m_login_window->set_state(login_window::state::INCORRECT_CREDENTIALS);
  } catch(const std::exception&) {
    m_login_window->set_state(login_window::state::SERVER_UNAVAILABLE);
  }
}
