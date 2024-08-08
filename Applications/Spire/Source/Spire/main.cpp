#include <filesystem>
#include <fstream>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <tclap/CmdLine.h>
#include "Nexus/TelemetryService/ApplicationDefinitions.hpp"
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
#include "Spire/Spire/SpireServiceClients.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Toolbar/ToolbarController.hpp"
#include "Version.hpp"
#include <QtPlugin>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Services;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace Nexus;
using namespace Nexus::TelemetryService;
using namespace Spire;
using namespace Spire::LegacyUI;

inline void InitializeResources() {
  Q_INIT_RESOURCE(LegacyResources);
}

namespace {
  template<typename C>
  using MetaTelemetryClient = TelemetryClient<C, TimeClientBox>;
  using SpireTelemetryClient = ApplicationClient<MetaTelemetryClient,
    ServiceName<TelemetryService::SERVICE_NAME>,
    ZLibSessionBuilder<ServiceLocatorClientBox>>;

  std::vector<SignInController::ServerEntry> ParseServers(
      const YAML::Node& config, const std::filesystem::path& configPath) {
    auto servers = std::vector<SignInController::ServerEntry>();
    if(!config["servers"]) {
      {
        auto configFile = std::ofstream(configPath);
        configFile <<
          "---\n"
          "servers:\n"
          "  - name: Local Environment\n"
          "    address: 127.0.0.1:20000\n"
          "...";
      }
      auto configStream = std::ifstream(configPath);
      if(!configStream.good()) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
          QObject::tr("Unable to load configuration: config.yml"));
      }
      return ParseServers(YAML::Load(configStream), configPath);
    }
    auto serverList = GetNode(config, "servers");
    for(auto server : serverList) {
      auto name = Extract<std::string>(server, "name");
      auto address = Extract<IpAddress>(server, "address");
      servers.push_back({name, address});
    }
    return servers;
  }
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
  freopen("stdout.log", "w", stdout);
  freopen("stderr.log", "w", stderr);
#endif
  auto show_sign_in_window = true;
  auto command_line = TCLAP::CmdLine("", ' ', "Spire " SPIRE_VERSION);
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
  try {
    command_line.parse(argc, argv);
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
  auto applicationPath =
    QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  auto configPath = std::filesystem::path(applicationPath.toStdString());
  if(!std::filesystem::exists(configPath)) {
    std::filesystem::create_directories(configPath);
  }
  configPath /= "config.yml";
  if(!std::filesystem::is_regular_file(configPath)) {
    auto configFile = std::ofstream(configPath);
    configFile <<
      "---\n"
      "servers:\n"
      "  - name: Local Environment\n"
      "    address: 127.0.0.1:20000\n"
      "...\n";
  }
  auto config = YAML::Node();
  try {
    auto configStream = std::ifstream(configPath);
    if(!configStream.good()) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load configuration: config.yml"));
      return -1;
    }
    config = YAML::Load(configStream);
  } catch(const YAML::ParserException&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  auto servers = std::vector<SignInController::ServerEntry>();
  try {
    servers = ParseServers(config, configPath);
  } catch(const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  auto user_profile = optional<UserProfile>();
  auto risk_timer_monitor = optional<RiskTimerMonitor>();
  auto toolbar_controller = optional<ToolbarController>();
  auto telemetry_client_mutex = Mutex();
  auto application_telemetry_client = std::unique_ptr<SpireTelemetryClient>();
  auto telemetry_client = std::unique_ptr<TelemetryClientBox>();
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
      auto service_clients = std::make_unique<SpireServiceClients>(
        std::move(service_locator_client));
      try {
        auto spire_telemetry_client =
          std::make_unique<SpireTelemetryClient>(
            service_clients->GetServiceLocatorClient(),
            service_clients->GetTimeClient());
        auto lock = std::lock_guard(telemetry_client_mutex);
        application_telemetry_client = std::move(spire_telemetry_client);
        telemetry_client = std::make_unique<TelemetryClientBox>(
          application_telemetry_client->Get());
      } catch(const std::exception&) {
        throw SignInException("Telemetry server not available.");
      }
      return ServiceClientsBox(std::move(service_clients));
    };
  auto sign_in_handler = [&] (auto service_clients) {
    auto is_administrator =
      service_clients.GetAdministrationClient().CheckAdministrator(
        service_clients.GetServiceLocatorClient().GetAccount());
    auto is_manager = is_administrator ||
      !service_clients.GetAdministrationClient().LoadManagedTradingGroups(
        service_clients.GetServiceLocatorClient().GetAccount()).empty();
    user_profile.emplace(
      service_clients.GetServiceLocatorClient().GetAccount().m_name,
      is_administrator, is_manager,
      service_clients.GetDefinitionsClient().LoadCountryDatabase(),
      service_clients.GetDefinitionsClient().LoadTimeZoneDatabase(),
      service_clients.GetDefinitionsClient().LoadCurrencyDatabase(),
      service_clients.GetDefinitionsClient().LoadExchangeRates(),
      service_clients.GetDefinitionsClient().LoadMarketDatabase(),
      service_clients.GetDefinitionsClient().LoadDestinationDatabase(),
      service_clients.GetAdministrationClient().LoadEntitlements(),
      get_default_additional_tag_database(), std::move(service_clients),
      *telemetry_client);
    auto sign_in_data = JsonObject();
    sign_in_data["version"] = std::string(SPIRE_VERSION);
    try {
      user_profile->CreateProfilePath();
    } catch(const std::exception&) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Error creating profile path."));
      return;
    }
    BlotterSettings::Load(Store(*user_profile));
    CatalogSettings::Load(Store(*user_profile));
    BookViewProperties::Load(Store(*user_profile));
    RiskTimerProperties::Load(Store(*user_profile));
    TimeAndSalesProperties::Load(Store(*user_profile));
    PortfolioViewerProperties::Load(Store(*user_profile));
    OrderImbalanceIndicatorProperties::Load(Store(*user_profile));
    SavedDashboards::Load(Store(*user_profile));
    toolbar_controller.emplace(Ref(*user_profile));
    toolbar_controller->open();
    risk_timer_monitor.emplace(Ref(*user_profile));
    risk_timer_monitor->Load();
  };
  if(show_sign_in_window) {
    auto sign_in_controller = SignInController(
      SPIRE_VERSION, std::move(servers), service_client_factory);
    sign_in_controller.open();
    sign_in_controller.connect_signed_in_signal(sign_in_handler);
  } else {
    try {
      auto service_clients = service_client_factory(
        username_argument.getValue(), password_argument.getValue(),
          IpAddress(host_argument.getValue(),
            static_cast<unsigned short>(port_argument.getValue())));
      sign_in_handler(std::move(service_clients));
      std::cout << "1" << std::flush;
    } catch(const std::exception& e) {
      std::cout << e.what() << std::flush;
    }
  }
  if(!user_profile) {
    return -1;
  }
  auto hotkey_override = HotkeyOverride();
  application.exec();
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
