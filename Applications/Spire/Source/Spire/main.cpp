#include <filesystem>
#include <fstream>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <QApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QStandardPaths>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/TelemetryService/ApplicationDefinitions.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/LoginDialog.hpp"
#include "Spire/LegacyUI/Toolbar.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Login/LoginController.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/RiskTimer/RiskTimerMonitor.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Spire/SpireServiceClients.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Version.hpp"
#include <QtPlugin>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Services;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace Nexus;
using namespace Nexus::TelemetryService;
using namespace Spire;
using namespace Spire::LegacyUI;

inline void InitializeResources() {
  Q_INIT_RESOURCE(Resources);
}

namespace {
  template<typename C>
  using MetaTelemetryClient = TelemetryClient<C, TimeClientBox>;
  using SpireTelemetryClient = ApplicationClient<MetaTelemetryClient,
    ServiceName<TelemetryService::SERVICE_NAME>,
    ZLibSessionBuilder<ServiceLocatorClientBox>>;

  std::vector<LoginController::ServerEntry> ParseServers(
      const YAML::Node& config, const std::filesystem::path& configPath) {
    auto servers = std::vector<LoginController::ServerEntry>();
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

  void LoadDefaultLayout(
      std::vector<QWidget*>& windows, UserProfile& userProfile) {
    auto instantiateSecurityWindows = true;
    auto nextPosition = QPoint(0, 0);
    auto nextHeight = 0;
    auto resolution = QGuiApplication::primaryScreen()->availableGeometry();
    auto defaultSecurities = std::vector<Security>();
    auto& marketEntry = userProfile.GetMarketDatabase().FromCode("XTSE");
    defaultSecurities.push_back(
      Security("RY", marketEntry.m_code, marketEntry.m_countryCode));
    defaultSecurities.push_back(
      Security("XIU", marketEntry.m_code, marketEntry.m_countryCode));
    defaultSecurities.push_back(
      Security("ABX", marketEntry.m_code, marketEntry.m_countryCode));
    defaultSecurities.push_back(
      Security("SU", marketEntry.m_code, marketEntry.m_countryCode));
    defaultSecurities.push_back(
      Security("BCE", marketEntry.m_code, marketEntry.m_countryCode));
    auto index = std::size_t(0);
    while(instantiateSecurityWindows && index < defaultSecurities.size()) {
      auto width = 0;
      auto bookViewWindow = new BookViewWindow(
        Ref(userProfile), userProfile.GetDefaultBookViewProperties(), "");
      auto timeAndSalesWindow = new TimeAndSalesWindow(
        Ref(userProfile), userProfile.GetDefaultTimeAndSalesProperties(), "");
      bookViewWindow->Link(*timeAndSalesWindow);
      timeAndSalesWindow->Link(*bookViewWindow);
      bookViewWindow->move(nextPosition);
      bookViewWindow->show();
      nextPosition.rx() += bookViewWindow->frameSize().width();
      width += bookViewWindow->frameSize().width();
      nextHeight = bookViewWindow->frameSize().height();
      timeAndSalesWindow->resize(150, bookViewWindow->height());
      timeAndSalesWindow->move(nextPosition);
      timeAndSalesWindow->show();
      bookViewWindow->DisplaySecurity(defaultSecurities[index]);
      nextPosition.rx() += timeAndSalesWindow->frameSize().width();
      width += timeAndSalesWindow->frameSize().width();
      windows.push_back(bookViewWindow);
      windows.push_back(timeAndSalesWindow);
      instantiateSecurityWindows = index < defaultSecurities.size() &&
        (nextPosition.x() + width < resolution.width());
      ++index;
    }
    nextPosition.setX(0);
    nextPosition.setY(nextHeight);
    auto toolbar = new Toolbar(Ref(userProfile));
    toolbar->move(nextPosition);
    toolbar->show();
    nextPosition.ry() += toolbar->frameSize().height();
    windows.push_back(toolbar);
    auto& globalBlotter = BlotterWindow::GetBlotterWindow(Ref(userProfile),
      Ref(userProfile.GetBlotterSettings().GetConsolidatedBlotter()));
    globalBlotter.move(nextPosition);
    globalBlotter.show();
    globalBlotter.resize(globalBlotter.width(),
      resolution.height() - nextPosition.y() -
      (globalBlotter.frameSize().height() - globalBlotter.size().height()));
    windows.push_back(&globalBlotter);
  }
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
  freopen("stdout.log", "w", stdout);
  freopen("stderr.log", "w", stderr);
#endif
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
  auto servers = std::vector<LoginController::ServerEntry>();
  try {
    servers = ParseServers(config, configPath);
  } catch(const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  auto login_controller = LoginController(SPIRE_VERSION, std::move(servers),
    [&] (const auto& username, const auto& password, const auto& address)  {
      return ServiceClientsBox(std::make_unique<SpireServiceClients>(
        std::make_unique<ApplicationServiceLocatorClient>(
          username, password, address)));
    });
  login_controller.open();
  login_controller.connect_logged_in_signal([&] (auto service_clients) {
  });
  application.exec();
/*
  auto loginDialog = LoginDialog(std::move(servers));
  auto loginResultCode = loginDialog.exec();
  if(loginResultCode == QDialog::Rejected) {
    return -1;
  }
  auto serviceClients = optional<ServiceClientsBox>();
  try {
    serviceClients.emplace(std::make_unique<SpireServiceClients>(
      loginDialog.GetServiceLocatorClient()));
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr(e.what()));
    return -1;
  }
*/
#if 0
  auto applicationTelemetryClient = optional<SpireTelemetryClient>();
  auto telemetryClient = optional<TelemetryClientBox>();
  try {
    applicationTelemetryClient.emplace(
      serviceClients->GetServiceLocatorClient(),
      serviceClients->GetTimeClient());
    telemetryClient.emplace(applicationTelemetryClient->Get());
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr(e.what()));
    return -1;
  }
  auto isAdministrator =
    serviceClients->GetAdministrationClient().CheckAdministrator(
      serviceClients->GetServiceLocatorClient().GetAccount());
  auto isManager = isAdministrator ||
    !serviceClients->GetAdministrationClient().LoadManagedTradingGroups(
      serviceClients->GetServiceLocatorClient().GetAccount()).empty();
  auto userProfile = UserProfile(loginDialog.GetUsername(), isAdministrator,
    isManager, serviceClients->GetDefinitionsClient().LoadCountryDatabase(),
    serviceClients->GetDefinitionsClient().LoadTimeZoneDatabase(),
    serviceClients->GetDefinitionsClient().LoadCurrencyDatabase(),
    serviceClients->GetDefinitionsClient().LoadExchangeRates(),
    serviceClients->GetDefinitionsClient().LoadMarketDatabase(),
    serviceClients->GetDefinitionsClient().LoadDestinationDatabase(),
    serviceClients->GetAdministrationClient().LoadEntitlements(),
    *serviceClients, *telemetryClient);
  auto loginData = JsonObject();
  loginData["version"] = std::string(SPIRE_VERSION);
  try {
    userProfile.CreateProfilePath();
  } catch(const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Error creating profile path."));
    return -1;
  }
  BlotterSettings::Load(Store(userProfile));
  CatalogSettings::Load(Store(userProfile));
  BookViewProperties::Load(Store(userProfile));
  RiskTimerProperties::Load(Store(userProfile));
  TimeAndSalesProperties::Load(Store(userProfile));
  PortfolioViewerProperties::Load(Store(userProfile));
  KeyBindings::Load(Store(userProfile));
  InteractionsProperties::Load(Store(userProfile));
  OrderImbalanceIndicatorProperties::Load(Store(userProfile));
  SavedDashboards::Load(Store(userProfile));
  auto windowSettings = WindowSettings::Load(userProfile);
  auto windows = std::vector<QWidget*>();
  auto hotkeyOverride = HotkeyOverride();
  if(!windowSettings.empty()) {
    for(auto& settings : windowSettings) {
      if(auto window = settings->Reopen(Ref(userProfile))) {
        windows.push_back(window);
      }
    }
  } else {
    LoadDefaultLayout(windows, userProfile);
  }
  for(auto& window : windows) {
    window->show();
  }
  auto riskMonitor = RiskTimerMonitor(Ref(userProfile));
  riskMonitor.Load();
  application.exec();
  SavedDashboards::Save(userProfile);
  OrderImbalanceIndicatorProperties::Save(userProfile);
  InteractionsProperties::Save(userProfile);
  KeyBindings::Save(userProfile);
  PortfolioViewerProperties::Save(userProfile);
  TimeAndSalesProperties::Save(userProfile);
  RiskTimerProperties::Save(userProfile);
  BookViewProperties::Save(userProfile);
  CatalogSettings::Save(userProfile);
  BlotterSettings::Save(userProfile);
#endif
  return 0;
}
