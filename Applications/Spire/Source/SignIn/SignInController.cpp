#include "Spire/SignIn/SignInController.hpp"
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/SignIn/SignInException.hpp"
#include "Spire/SignIn/SignInWindow.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SignInController::SignInController(
  std::string version, std::vector<ServerEntry> servers,
  ServiceClientsFactory service_clients_factory)
  : m_version(std::move(version)),
    m_servers(std::move(servers)),
    m_service_clients_factory(std::move(service_clients_factory)),
    m_sign_in_window(nullptr) {}

void SignInController::open() {
  auto servers = std::vector<std::string>();
  std::transform(m_servers.begin(), m_servers.end(),
    std::back_inserter(servers), [] (const auto& server) {
      return server.m_name;
    });
  auto tracks = std::vector<Track>();
  tracks.push_back(Track::CURRENT);
  auto track = std::make_shared<LocalTrackModel>(Track::CURRENT);
  m_sign_in_window = new SignInWindow(
    m_version, std::move(tracks), std::move(track), std::move(servers));
  m_sign_in_window->connect_sign_in_signal(
    std::bind_front(&SignInController::on_sign_in, this));
  m_sign_in_window->connect_cancel_signal(
    std::bind_front(&SignInController::on_cancel, this));
  m_sign_in_window->show();
}

connection SignInController::connect_signed_in_signal(
    const SignedInSignal::slot_type& slot) const {
  return m_signed_in_signal.connect(slot);
}

void SignInController::on_sign_in(const std::string& username,
    const std::string& password, Track track, const std::string& server) {
  m_sign_in_promise = QtPromise([=] {
    auto address = [&] {
      if(server.empty() && !m_servers.empty()) {
        return m_servers.front().m_address;
      }
      for(auto& entry : m_servers) {
        if(entry.m_name == server) {
          return entry.m_address;
        }
      }
      throw SignInException("Server not found.");
    }();
    return m_service_clients_factory(username, password, address);
  }, LaunchPolicy::ASYNC).then(
    std::bind_front(&SignInController::on_sign_in_promise, this));
}

void SignInController::on_cancel() {
  m_sign_in_promise.disconnect();
  m_sign_in_window->set_state(SignInWindow::State::NONE);
}

void SignInController::on_sign_in_promise(
    Expect<ServiceClientsBox> service_clients) {
  if(service_clients.IsException()) {
    try {
      std::rethrow_exception(service_clients.GetException());
    } catch(const AuthenticationException&) {
      m_sign_in_window->set_error(
        QObject::tr("Incorrect username or password."));
    } catch(const SignInException& e) {
      m_sign_in_window->set_error(QObject::tr(e.what()));
    } catch(const std::exception&) {
      m_sign_in_window->set_error(QObject::tr("Server unavailable."));
    }
    return;
  }
  m_sign_in_window->close();
  delete_later(m_sign_in_window);
  m_signed_in_signal(std::move(service_clients.Get()));
}
