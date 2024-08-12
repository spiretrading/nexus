#include "Spire/SignIn/SignInController.hpp"
#include <filesystem>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/WebServices/HttpClient.hpp>
#include <Beam/WebServices/TcpChannelFactory.hpp>
#include <QFileInfo>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/SignIn/SignInException.hpp"
#include "Spire/SignIn/SignInWindow.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::WebServices;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto parse_directories(const std::string& html) {
    auto directories = std::vector<std::string>();
    auto start = html.find("<pre>");
    if(start == std::string::npos) {
      return directories;
    }
    start += std::strlen("<pre>");
    auto end = html.find("</pre>", start);
    if(end == std::string::npos) {
      return directories;
    }
    auto content = html.substr(start, end - start);
    auto pos = std::size_t(0);
    while((pos = content.find("<a href=\"", pos)) != std::string::npos) {
      pos += std::strlen("<a href=\"");
      auto end_quote = content.find("\"", pos);
      if(end_quote == std::string::npos) {
        break;
      }
      auto directory = content.substr(pos, end_quote - pos);
      if(!directory.empty() && directory != "../" && directory.back() == '/') {
        directories.push_back(directory.substr(0, directory.size() - 1));
      }
      pos = end_quote + 1;
    }
    return directories;
  }

  auto load_latest_build(
      const IpAddress& address, Track track, const std::string& version) {
    auto uri = "http://" + address.GetHost() + ":8080" + "/spire/" +
      to_text(track).toLower().toStdString() + "/x86/windows/";
    auto request = HttpRequest(Uri(uri));
    auto directory_listing = std::string();
    try {
      auto client =
        HttpClient<std::unique_ptr<ChannelBox>>(TcpSocketChannelFactory());
      auto response = client.Send(request);
      if(response.GetStatusCode() != HttpStatusCode::OK) {
        return version;
      }
      directory_listing =
        std::string(response.GetBody().GetData(), response.GetBody().GetSize());
    } catch(const std::exception&) {
      return version;
    }
    auto directories = parse_directories(directory_listing);
    try {
      auto latest_build = std::stoi(version);
      for(auto& directory : directories) {
        try {
          latest_build = std::max(latest_build, std::stoi(directory));
        } catch(const std::exception&) {
        }
      }
      return std::to_string(latest_build);
    } catch(const std::exception&) {
      return version;
    }
  }

  auto update_build(
      const IpAddress& address, Track track, const std::string& build) {
    auto uri = "http://" + address.GetHost() + ":8080" + "/spire/" +
      to_text(track).toLower().toStdString() + "/x86/windows/" + build +
      "/Spire.exe";
    auto request = HttpRequest(Uri(uri));
    auto directory_listing = std::string();
    try {
      auto client =
        HttpClient<std::unique_ptr<ChannelBox>>(TcpSocketChannelFactory());
      auto response = client.Send(request);
      if(response.GetStatusCode() != HttpStatusCode::OK) {
        return;
      }
      auto executable_path = std::filesystem::canonical(std::filesystem::path(
        QCoreApplication::applicationFilePath().toStdString()));
      std::filesystem::rename(
        executable_path, executable_path.filename().string() + ".old");
      auto out_file = std::ofstream(executable_path, std::ios::binary);
      out_file.write(
        response.GetBody().GetData(), response.GetBody().GetSize());
      if(!out_file) {
        qDebug() << QString::fromStdString(std::strerror(errno));
      }
    } catch(const std::exception&) {
      return;
    }
  }
}

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
  m_sign_in_promise = QtPromise([=] {
    auto latest_build = load_latest_build(address, track, m_version);
    if(latest_build != m_version) {
      update_build(address, track, latest_build);
    }
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
