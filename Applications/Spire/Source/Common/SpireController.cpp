#include "Spire/Spire/SpireController.hpp"
#ifdef slots
  #undef slots
#endif
#if 0
#include <filesystem>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include <QMessageBox>
#include <QStandardPaths>
#include "Spire/Login/LoginController.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Spire;

SpireController::SpireController()
  : m_state(State::NONE) {}

SpireController::~SpireController() = default;

void SpireController::open() {
  if(m_state != State::NONE) {
    return;
  }
  auto service_clients_factory =
    [=] (const auto& username, const auto& password, const auto& address) {
      return ServiceClientsBox(std::in_place_type<ApplicationServiceClients>,
        username, password, address);
    };
  auto server_entries = load_server_entries();
  if(server_entries.empty()) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return;
  }
  m_login_controller = std::make_unique<LoginController>(
    std::move(server_entries), service_clients_factory);
  m_login_controller->connect_logged_in_signal(
    [=] (const auto& definitions) { on_login(definitions); });
  m_state = State::LOGIN;
  m_login_controller->open();
}

std::vector<LoginController::ServerEntry>
    SpireController::load_server_entries() {
  auto application_path = QStandardPaths::writableLocation(
    QStandardPaths::DataLocation);
  auto config_path = std::filesystem::path(application_path.toStdString());
  if(!std::filesystem::exists(config_path)) {
    std::filesystem::create_directories(config_path);
  }
  config_path /= "config.yml";
  if(!std::filesystem::is_regular_file(config_path)) {
    auto config_file = std::ofstream(config_path);
    config_file <<
      "---\n"
      "servers:\n"
      "  - name: Local Environment\n"
      "    address: 127.0.0.1:20000\n"
      "...";
  }
  auto config = YAML::Node();
  try {
    auto config_stream = std::ifstream(config_path);
    if(!config_stream.good()) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load configuration: config.yml"));
      return {};
    }
    config = YAML::Load(config_stream);
  } catch(const YAML::ParserException&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return {};
  }
  auto servers = std::vector<LoginController::ServerEntry>();
  if(!config["servers"]) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to load configuration: config.yml"));
    return {};
  }
  auto server_list = GetNode(config, "servers");
  for(auto server : server_list) {
    auto name = Extract<std::string>(server, "name");
    auto address = Extract<IpAddress>(server, "address");
    servers.push_back({name, address});
  }
  return servers;
}

void SpireController::on_login(const Definitions& definitions) {
  m_state = State::TOOLBAR;
}
#endif
