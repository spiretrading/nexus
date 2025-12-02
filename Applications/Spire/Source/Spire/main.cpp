#include <filesystem>
#include <fstream>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QStandardPaths>
#include <tclap/CmdLine.h>
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/KeyBindings/KeyBindingsProfile.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/RiskTimer/RiskTimerMonitor.hpp"
#include "Spire/SignIn/SignInController.hpp"
#include "Spire/SignIn/SignInException.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/SpireClients.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Toolbar/ToolbarController.hpp"
#include "Version.hpp"
#include <QtPlugin>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

inline void InitializeResources() {
  Q_INIT_RESOURCE(LegacyResources);
}

namespace {
  std::vector<SignInController::ServerEntry> parse_servers(
      const YAML::Node& config, const std::filesystem::path& config_path) {
    auto servers = std::vector<SignInController::ServerEntry>();
    if(!config["servers"]) {
      {
        auto config_file = std::ofstream(config_path);
        config_file <<
          "---\n"
          "servers:\n"
          "  - name: Local Environment\n"
          "    address: 127.0.0.1:20000\n"
          "...";
      }
      auto config_stream = std::ifstream(config_path);
      if(!config_stream.good()) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
          QObject::tr("Unable to load configuration: config.yml"));
      }
      return parse_servers(YAML::Load(config_stream), config_path);
    }
    auto server_list = get_node(config, "servers");
    for(auto server : server_list) {
      auto name = extract<std::string>(server, "name");
      auto address = extract<IpAddress>(server, "address");
      servers.push_back({name, address});
    }
    return servers;
  }

  void write_to_shared_memory(
      const std::string& key, const std::string& message) {
    auto memory = QSharedMemory(QString::fromStdString(key));
    if(!memory.attach()) {
      return;
    }
    memory.lock();
    std::strcpy(static_cast<char*>(memory.data()), message.c_str());
    memory.unlock();
  }
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
  freopen("stdout.log", "w", stdout);
  freopen("stderr.log", "w", stderr);
#endif
  auto show_sign_in_window = true;
  auto command_line = TCLAP::CmdLine("", ' ', "Spire " SPIRE_VERSION);
  auto key_argument =
    TCLAP::ValueArg<std::string>("k", "key", "Shared key", false, "", "text");
  command_line.add(key_argument);
  auto username_argument = TCLAP::ValueArg<std::string>(
    "u", "username", "Username", false, "", "text");
  command_line.add(username_argument);
  auto password_argument = TCLAP::ValueArg<std::string>(
    "p", "password", "Password", false, "", "text");
  command_line.add(password_argument);
  auto host_argument =
    TCLAP::ValueArg<std::string>("a", "address", "Address", false, "", "text");
  command_line.add(host_argument);
  auto port_argument =
    TCLAP::ValueArg<int>("s", "port", "Port", false, 0, "integer");
  command_line.add(port_argument);
  auto build_argument = TCLAP::SwitchArg("b", "build", "Build");
  command_line.add(build_argument);
  try {
    command_line.parse(argc, argv);
    if(build_argument.getValue() && key_argument.isSet()) {
      write_to_shared_memory(key_argument.getValue(), SPIRE_VERSION "\n");
      return 0;
    }
    show_sign_in_window = !username_argument.isSet() ||
      !password_argument.isSet() || !host_argument.isSet() ||
      !port_argument.isSet();
  } catch(const TCLAP::ArgException& e) {
    std::cout << "Error parsing command line: " + e.error() + " for argument " +
      e.argId() << std::flush;
  }
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading"));
  application.setApplicationName(QObject::tr("Spire"));
  application.setApplicationVersion(SPIRE_VERSION);
  initialize_resources();
  RegisterCustomQtVariants();
  InitializeResources();
  auto application_path =
    QStandardPaths::writableLocation(QStandardPaths::DataLocation);
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
      "...\n";
  }
  auto config = YAML::Node();
  try {
    auto config_stream = std::ifstream(config_path);
    if(!config_stream.good()) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load configuration: config.yml"));
      return -1;
    }
    config = YAML::Load(config_stream);
  } catch(const YAML::ParserException&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  auto servers = std::vector<SignInController::ServerEntry>();
  try {
    servers = parse_servers(config, config_path);
  } catch(const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  auto user_profile = optional<UserProfile>();
  auto risk_timer_monitor = optional<RiskTimerMonitor>();
  auto toolbar_controller = optional<ToolbarController>();
  auto service_client_factory =
    [&] (const auto& username, const auto& password, const auto& address)  {
      auto service_locator_client =
        std::unique_ptr<ApplicationServiceLocatorClient>();
      try {
        service_locator_client =
          std::make_unique<ApplicationServiceLocatorClient>(
            username, password, address);
      } catch(const std::exception& e) {
        try {
          std::rethrow_if_nested(e);
        } catch(const std::exception& e) {
          if(std::string(e.what()) == "Invalid username or password.") {
            throw AuthenticationException();
          }
        }
        throw;
      }
      auto clients =
        std::make_unique<SpireClients>(std::move(service_locator_client));
      return Clients(std::move(clients));
    };
  auto sign_in_controller = std::unique_ptr<SignInController>();
  auto sign_in_handler = [&] (auto clients) {
    try {
      auto is_administrator = [&] {
        try {
          return clients.get_administration_client().check_administrator(
            clients.get_service_locator_client().get_account());
        } catch(const std::exception&) {
          throw std::runtime_error("Unable to verify administrator status.");
        }
      }();
      auto is_manager = [&] {
        try {
          return is_administrator ||
            !clients.get_administration_client().load_managed_trading_groups(
              clients.get_service_locator_client().get_account()).empty();
        } catch(const std::exception&) {
          throw std::runtime_error("Unable to verify manager status.");
        }
      }();
      auto exchange_rates = [&] {
        try {
          return clients.get_definitions_client().load_exchange_rates();
        } catch(const std::exception&) {
          throw std::runtime_error("Unable to load exchange rates.");
        }
      }();
      auto entitlements = [&] {
        try {
          return clients.get_administration_client().load_entitlements();
        } catch(const std::exception&) {
          throw std::runtime_error("Unable to load entitlements.");
        }
      }();
      user_profile.emplace(
        clients.get_service_locator_client().get_account().m_name,
        is_administrator, is_manager, std::move(exchange_rates),
        std::move(entitlements), get_default_additional_tag_database(),
        std::move(clients));
      auto sign_in_data = JsonObject();
      sign_in_data["version"] = std::string(SPIRE_VERSION);
      try {
        user_profile->CreateProfilePath();
      } catch(const std::exception&) {
        throw std::runtime_error("Unable to create profile path.");
      }
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr(e.what()));
      return;
    }
    BlotterSettings::Load(out(*user_profile));
    CatalogSettings::Load(out(*user_profile));
    BookViewProperties::Load(out(*user_profile));
    RiskTimerProperties::Load(out(*user_profile));
    TimeAndSalesProperties::Load(out(*user_profile));
    PortfolioViewerProperties::Load(out(*user_profile));
    OrderImbalanceIndicatorProperties::Load(out(*user_profile));
    SavedDashboards::Load(out(*user_profile));
    toolbar_controller.emplace(Ref(*user_profile));
    toolbar_controller->open();
    risk_timer_monitor.emplace(Ref(*user_profile));
    risk_timer_monitor->Load();
    sign_in_controller = nullptr;
  };
  if(show_sign_in_window) {
    sign_in_controller = std::make_unique<SignInController>(
      SPIRE_VERSION, std::move(servers), service_client_factory);
    sign_in_controller->open();
    sign_in_controller->connect_signed_in_signal(sign_in_handler);
  } else {
    try {
      auto service_clients = service_client_factory(
        username_argument.getValue(), password_argument.getValue(),
          IpAddress(host_argument.getValue(),
            static_cast<unsigned short>(port_argument.getValue())));
      sign_in_handler(std::move(service_clients));
      if(key_argument.isSet()) {
        write_to_shared_memory(key_argument.getValue(), "1\n");
      }
    } catch(const std::exception& e) {
      if(key_argument.isSet()) {
        write_to_shared_memory(
          key_argument.getValue(), std::string(e.what()) + "\n");
      }
      return -1;
    }
  }
  auto hotkey_override = HotkeyOverride();
  application.exec();
  if(!user_profile) {
    return -1;
  }
  SavedDashboards::Save(*user_profile);
  OrderImbalanceIndicatorProperties::Save(*user_profile);
  save_key_bindings_profile(
    *user_profile->GetKeyBindings(), user_profile->GetProfilePath());
  PortfolioViewerProperties::Save(*user_profile);
  TimeAndSalesProperties::Save(*user_profile);
  RiskTimerProperties::Save(*user_profile);
  BookViewProperties::Save(*user_profile);
  CatalogSettings::Save(*user_profile);
  BlotterSettings::Save(*user_profile);
  toolbar_controller->close();
  return 0;
}
