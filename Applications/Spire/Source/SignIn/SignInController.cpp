#include "Spire/SignIn/SignInController.hpp"
#include <filesystem>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/WebServices/HttpClient.hpp>
#include <Beam/WebServices/TcpChannelFactory.hpp>
#include <QProcess>
#include <QSharedMemory>
#include <QStandardPaths>
#include <QThread>
#include <QUuid>
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
  std::size_t index(Track track) {
    return static_cast<std::underlying_type_t<Track>>(track);
  }

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

  auto get_update_url(
      const IpAddress& address, Track track, const std::string& path) {
    return Uri("http://" + address.GetHost() + ":8080" +
      "/distribution/spire/" + to_text(track).toLower().toStdString() +
      "/x86/windows/" + path);
  }

  auto load_latest_build(
      const IpAddress& address, Track track, const std::string& version) {
    auto request = HttpRequest(get_update_url(address, track, ""));
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

  std::filesystem::path get_application_filename(Track track) {
    if(track == Track::CURRENT) {
      return "Spire.exe";
    }
    return "Spire." + to_text(track).toLower().toStdString() + ".exe";
  }

  std::filesystem::path get_application_path(Track track) {
    return std::filesystem::weakly_canonical(std::filesystem::path(
      QCoreApplication::applicationFilePath().toStdString()).parent_path() /
      get_application_filename(track));
  }

  std::string launch_and_read(Track track, QStringList arguments) {
    auto memory_key = QUuid::createUuid().toString();
    arguments << "-k" << memory_key;
    auto memory = QSharedMemory(memory_key);
    if(!memory.create(1024)) {
      throw std::runtime_error("Failed to load shared memory.");
    }
    std::memset(memory.data(), 0, memory.size());
    auto child_process = QProcess();
    child_process.setProgram(
      QString::fromStdString(get_application_path(track).string()));
    child_process.setArguments(arguments);
    if(!child_process.startDetached()) {
      throw std::runtime_error("Unable to start process.");
    }
    while(true) {
      memory.lock();
      if(static_cast<const char*>(memory.data())[0] != '\0') {
        auto output =
          QString::fromUtf8(static_cast<const char*>(memory.data()));
        memory.unlock();
        if(output.endsWith('\n')) {
          output.chop(1);
          return output.toStdString();
        }
        throw std::runtime_error("Unable to read output.");
      }
      memory.unlock();
      QThread::msleep(100);
    }
  }

  bool launch_update(const IpAddress& address, Track track,
      const std::string& username, const std::string& password) {
    auto arguments = QStringList();
    arguments << "-u" << QString::fromStdString(username) << "-p" <<
      QString::fromStdString(password) << "-a" <<
      QString::fromStdString(address.GetHost()) << "-s" <<
      QString::number(address.GetPort());
    auto output = std::string();
    try {
      output = launch_and_read(track, arguments);
    } catch(const std::exception&) {
      return false;
    }
    if(output == "1") {
      return true;
    }
    if(output == "Unable to authenticate connection.") {
      throw AuthenticationException();
    } else if(output == "Server unavailable.") {
      throw std::runtime_error(output);
    } else {
      throw SignInException(output);
    }
  }

  std::filesystem::path load_temporary_directory() {
    #ifdef _WIN32
      auto length = GetTempPathA(0, nullptr);
      if(length == 0) {
        return QCoreApplication::applicationFilePath().toStdString();
      }
      auto path = std::make_unique<char[]>(length);
      GetTempPathA(length, path.get());
      return std::string(path.get());
    #endif
  }

  bool update_build(
      const IpAddress& address, Track track, const std::string& username,
      const std::string& password, const std::string& build) {
    auto request = HttpRequest(get_update_url(
      address, track, build + "/" + get_application_filename(track).string()));
    auto directory_listing = std::string();
    try {
      auto client =
        HttpClient<std::unique_ptr<ChannelBox>>(TcpSocketChannelFactory());
      auto response = client.Send(request);
      if(response.GetStatusCode() != HttpStatusCode::OK) {
        return false;
      }
      auto executable_path = get_application_path(track);
      try {
        std::filesystem::rename(executable_path,
          load_temporary_directory() / executable_path.filename());
      } catch(const std::filesystem::filesystem_error& e) {
        if(e.code() != std::errc::no_such_file_or_directory) {
          return false;
        }
      }
      {
        auto out_file = std::ofstream(executable_path, std::ios::binary);
        out_file.write(
          response.GetBody().GetData(), response.GetBody().GetSize());
      }
      return launch_update(address, track, username, password);
    } catch(const std::exception&) {
      return false;
    }
  }

  std::string load_version(Track track, const std::string& current_version) {
    if(track == Track::CURRENT) {
      return current_version;
    }
    try {
      auto arguments = QStringList();
      arguments << "-b";
      return launch_and_read(track, arguments);
    } catch(const std::exception&) {
      return "0";
    }
  }

  std::filesystem::path get_track_path() {
    auto path = std::filesystem::path(QStandardPaths::writableLocation(
      QStandardPaths::DataLocation).toStdString());
    return path / "track";
  }

  Track load_track() {
    auto file = std::ifstream(get_track_path());
    if(!file.is_open()) {
      return Track::CURRENT;
    }
    auto line = std::string();
    if(std::getline(file, line)) {
      if(line == "Classic") {
        return Track::CLASSIC;
      } else if(line == "Preview") {
        return Track::PREVIEW;
      }
    }
    return Track::CURRENT;
  }

  void store_track(Track track) {
    auto file = std::ofstream(get_track_path());
    if(!file.is_open()) {
      return;
    }
    file << to_text(track).toStdString();
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
  tracks.push_back(Track::CLASSIC);
  tracks.push_back(Track::PREVIEW);
  auto track = std::make_shared<LocalTrackModel>(load_track());
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
  store_track(track);
  m_sign_in_promise = QtPromise([=] () -> optional<ServiceClientsBox> {
    if(m_run_update.test(index(track))) {
      if(launch_update(address, track, username, password)) {
        return none;
      }
    } else {
      auto latest_build = load_latest_build(address, track, m_version);
      auto version = load_version(track, m_version);
      if(latest_build != version) {
        m_run_update.set(index(track));
        if(update_build(address, track, username, password, latest_build)) {
          return none;
        }
      } else if(track != Track::CURRENT) {
        if(launch_update(address, track, username, password)) {
          return none;
        }
      }
    }
    return m_service_clients_factory(username, password, address);
  }, LaunchPolicy::ASYNC).then(
    [=] (Expect<optional<ServiceClientsBox>> service_clients) {
      if(service_clients.IsException()) {
        on_sign_in_promise(
          Expect<ServiceClientsBox>(service_clients.GetException()));
      } else if(service_clients.Get()) {
        on_sign_in_promise(std::move(*service_clients.Get()));
      } else {
        m_sign_in_window->close();
        delete_later(m_sign_in_window);
      }
    });
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
