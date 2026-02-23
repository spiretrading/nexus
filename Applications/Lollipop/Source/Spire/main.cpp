#include <filesystem>
#include <fstream>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <QApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QSharedMemory>
#include <QStandardPaths>
#include <tclap/CmdLine.h>
#include "Nexus/Clients/ServiceClients.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/RiskTimer/RiskTimerMonitor.hpp"
#include "Spire/Spire/SpireClients.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/LoginDialog.hpp"
#include "Spire/UI/Toolbar.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "Spire/UI/WindowSettings.hpp"
#include "Version.hpp"
#include <QtPlugin>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

inline void InitializeResources() {
  Q_INIT_RESOURCE(Resources);
}

namespace {
  std::vector<LoginDialog::ServerEntry> ParseServers(
      const YAML::Node& config, const std::filesystem::path& configPath) {
    auto servers = std::vector<LoginDialog::ServerEntry>();
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
    auto serverList = get_node(config, "servers");
    for(auto server : serverList) {
      auto name = extract<std::string>(server, "name");
      auto address = extract<IpAddress>(server, "address");
      servers.push_back({name, address});
    }
    return servers;
  }

  void LoadDefaultLayout(
      std::vector<QWidget*>& windows, UserProfile& userProfile) {
    auto instantiateTickerWindows = true;
    auto nextPosition = QPoint(0, 0);
    auto nextHeight = 0;
    auto resolution = QGuiApplication::primaryScreen()->availableGeometry();
    auto defaultTickers = std::vector<Ticker>();
    auto& venueEntry = userProfile.GetVenueDatabase().from("XTSE");
    defaultTickers.push_back(Ticker("RY", venueEntry.m_venue));
    defaultTickers.push_back(Ticker("XIU", venueEntry.m_venue));
    defaultTickers.push_back(Ticker("ABX", venueEntry.m_venue));
    defaultTickers.push_back(Ticker("SU", venueEntry.m_venue));
    defaultTickers.push_back(Ticker("BCE", venueEntry.m_venue));
    auto index = std::size_t(0);
    while(instantiateTickerWindows && index < defaultTickers.size()) {
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
      bookViewWindow->DisplayTicker(defaultTickers[index]);
      nextPosition.rx() += timeAndSalesWindow->frameSize().width();
      width += timeAndSalesWindow->frameSize().width();
      windows.push_back(bookViewWindow);
      windows.push_back(timeAndSalesWindow);
      instantiateTickerWindows = index < defaultTickers.size() &&
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

  void WriteToSharedMemory(const std::string& key, const std::string& message) {
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
  auto showSignInWindow = true;
  auto commandLine = TCLAP::CmdLine("", ' ', "Spire " SPIRE_VERSION);
  auto keyArgument = TCLAP::ValueArg<std::string>(
    "k", "key", "Shared key", false, "", "text");
  commandLine.add(keyArgument);
  auto usernameArgument = TCLAP::ValueArg<std::string>(
    "u", "username", "Username", false, "", "text");
  commandLine.add(usernameArgument);
  auto passwordArgument = TCLAP::ValueArg<std::string>(
    "p", "password", "Password", false, "", "text");
  commandLine.add(passwordArgument);
  auto hostArgument =
    TCLAP::ValueArg<std::string>("a", "address", "Address", false, "", "text");
  commandLine.add(hostArgument);
  auto portArgument =
    TCLAP::ValueArg<int>("s", "port", "Port", false, 0, "integer");
  commandLine.add(portArgument);
  auto buildArgument = TCLAP::SwitchArg("b", "build", "Build");
  commandLine.add(buildArgument);
  try {
    commandLine.parse(argc, argv);
    if(buildArgument.getValue() && keyArgument.isSet()) {
      WriteToSharedMemory(keyArgument.getValue(), SPIRE_VERSION "\n");
      return 0;
    }
    showSignInWindow = !usernameArgument.isSet() ||
      !passwordArgument.isSet() || !hostArgument.isSet() ||
      !portArgument.isSet();
  } catch(const TCLAP::ArgException& e) {
    std::cout << "Error parsing command line: " + e.error() + " for argument " +
      e.argId() << std::flush;
  }
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading"));
  application.setApplicationName(QObject::tr("Spire"));
  application.setApplicationVersion(SPIRE_VERSION);
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
  auto servers = std::vector<LoginDialog::ServerEntry>();
  try {
    servers = ParseServers(config, configPath);
  } catch(const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  auto serviceClients = optional<Clients>();
  if(showSignInWindow) {
    auto loginDialog = LoginDialog(std::move(servers));
    auto loginResultCode = loginDialog.exec();
    if(loginResultCode == QDialog::Rejected) {
      return -1;
    }
    try {
      serviceClients.emplace(std::make_unique<SpireClients>(
        loginDialog.GetServiceLocatorClient()));
    } catch(const std::exception& e) {
      QMessageBox::critical(
        nullptr, QObject::tr("Error"), QObject::tr(e.what()));
      return -1;
    }
  } else {
    try {
      auto loader = QtPromise([&] {
        auto service_locator_client =
          std::unique_ptr<ApplicationServiceLocatorClient>();
        try {
          service_locator_client =
            std::make_unique<ApplicationServiceLocatorClient>(
              usernameArgument.getValue(), passwordArgument.getValue(),
              IpAddress(hostArgument.getValue(),
                static_cast<unsigned short>(portArgument.getValue())));
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
        auto service_clients = std::make_unique<SpireClients>(
          std::move(service_locator_client));
        return Clients(std::move(service_clients));
      }, LaunchPolicy::ASYNC);
      serviceClients.emplace(wait(std::move(loader)));
      if(keyArgument.isSet()) {
        WriteToSharedMemory(keyArgument.getValue(), "1\n");
      }
    } catch(const std::exception& e) {
      if(keyArgument.isSet()) {
        WriteToSharedMemory(
          keyArgument.getValue(), std::string(e.what()) + "\n");
      }
      return -1;
    }
  }
  auto isAdministrator =
    serviceClients->get_administration_client().check_administrator(
      serviceClients->get_service_locator_client().get_account());
  auto isManager = isAdministrator ||
    !serviceClients->get_administration_client().load_managed_trading_groups(
      serviceClients->get_service_locator_client().get_account()).empty();
  auto userProfile = UserProfile(
    serviceClients->get_service_locator_client().get_account().m_name,
    isAdministrator, isManager,
    serviceClients->get_definitions_client().load_country_database(),
    serviceClients->get_definitions_client().load_time_zone_database(),
    serviceClients->get_definitions_client().load_currency_database(),
    serviceClients->get_definitions_client().load_exchange_rates(),
    serviceClients->get_definitions_client().load_venue_database(),
    serviceClients->get_definitions_client().load_destination_database(),
    serviceClients->get_administration_client().load_entitlements(),
    *serviceClients);
  auto loginData = JsonObject();
  loginData["version"] = std::string(SPIRE_VERSION);
  try {
    userProfile.CreateProfilePath();
  } catch(const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Error creating profile path."));
    return -1;
  }
  BlotterSettings::Load(out(userProfile));
  CatalogSettings::Load(out(userProfile));
  BookViewProperties::Load(out(userProfile));
  RiskTimerProperties::Load(out(userProfile));
  TimeAndSalesProperties::Load(out(userProfile));
  PortfolioViewerProperties::Load(out(userProfile));
  KeyBindings::Load(out(userProfile));
  InteractionsProperties::Load(out(userProfile));
  OrderImbalanceIndicatorProperties::Load(out(userProfile));
  SavedDashboards::Load(out(userProfile));
  auto windowSettings = WindowSettings::Load(userProfile);
  auto windows = std::vector<QWidget*>();
  auto hotkey_override = HotkeyOverride();
  application.installNativeEventFilter(&hotkey_override);
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
  return 0;
}
