#include "spire/spire/spire_controller.hpp"
#include <filesystem>
#include <Beam/Network/SocketThreadPool.hpp>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include <QMessageBox>
#include <QStandardPaths>
#include "spire/login/login_controller.hpp"
#include "spire/toolbar/toolbar_controller.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Spire;

SpireController::SpireController()
    : m_state(State::NONE),
      m_socket_thread_pool(std::make_unique<SocketThreadPool>()),
      m_timer_thread_pool(std::make_unique<TimerThreadPool>()) {}

SpireController::~SpireController() = default;

void SpireController::open() {
  if(m_state != State::NONE) {
    return;
  }
  auto ip_address = load_ip_address();
  if(!ip_address.has_value()) {
    return;
  }
  auto service_clients_factory =
    [=] (auto&& username, auto&& password) {
      return MakeVirtualServiceClients(
        std::make_unique<ApplicationServiceClients>(*ip_address, username,
        password, Ref(*m_socket_thread_pool), Ref(*m_timer_thread_pool)));
    };
  m_login_controller = std::make_unique<LoginController>(
    service_clients_factory);
  m_login_controller->connect_logged_in_signal([=]{on_login();});
  m_state = State::LOGIN;
  m_login_controller->open();
}

std::optional<IpAddress> SpireController::load_ip_address() {
  auto application_path = QStandardPaths::writableLocation(
    QStandardPaths::DataLocation);
  std::filesystem::path config_path = application_path.toStdString();
  if(!std::filesystem::exists(config_path)) {
    std::filesystem::create_directories(config_path);
  }
  config_path /= "config.yml";
  if(!std::filesystem::is_regular_file(config_path)) {
    std::ofstream config_file(config_path);
    config_file <<
      "---\n"
      "address: 127.0.0.1:20000\n"
      "...\n";
  }
  YAML::Node config;
  try {
    std::ifstream config_stream(config_path);
    if(!config_stream.good()) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load configuration: config.yml"));
      return std::nullopt;
    }
    config = YAML::Load(config_stream);
  } catch(const YAML::ParserException&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return std::nullopt;
  }
  IpAddress address;
  try {
    address = Extract<IpAddress>(config, "address");
  } catch(const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return std::nullopt;
  }
  return address;
}

void SpireController::on_login() {
  m_service_clients = std::move(m_login_controller->get_service_clients());
  m_login_controller.reset();
  m_toolbar_controller = std::make_unique<ToolbarController>(
    *m_service_clients);
  m_toolbar_controller->open();
  m_state = State::TOOLBAR;
}
