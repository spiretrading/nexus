#include <fstream>
#include <iostream>
#include <sstream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/TimeService.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/Compliance/BuyingPowerComplianceRule.hpp"
#include "Nexus/Compliance/CancelRestrictionPeriodComplianceRule.hpp"
#include "Nexus/Compliance/OpposingOrderCancellationComplianceRule.hpp"
#include "Nexus/Compliance/OpposingOrderSubmissionComplianceRule.hpp"
#include "Nexus/Compliance/OrderCountPerSideComplianceRule.hpp"
#include "Nexus/Compliance/SubmissionRestrictionPeriodComplianceRule.hpp"
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"
#include "Nexus/DefinitionsService/DefinitionsServlet.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::DefinitionsService;
using namespace std;
using namespace TCLAP;

namespace {
  using DefinitionsServletContainer =
    ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
    MetaDefinitionsServlet, ApplicationServiceLocatorClient::Client*>,
    TcpServerSocket, BinarySender<SharedBuffer>, NullEncoder,
    std::shared_ptr<LiveTimer>>;

  struct DefinitionsServerConnectionInitializer {
    string m_serviceName;
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void DefinitionsServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<string>(config, "service",
      DefinitionsService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }

  vector<ExchangeRate> ParseExchangeRates(const YAML::Node& config) {
    vector<ExchangeRate> exchangeRates;
    for(auto& entry : config) {
      auto symbol = Extract<string>(entry, "symbol");
      auto pair = ParseCurrencyPair(symbol);
      auto rate = Extract<rational<int>>(entry, "rate");
      auto exchangeRate = ExchangeRate(pair, rate);
      exchangeRates.push_back(exchangeRate);
    }
    return exchangeRates;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" DEFINITIONS_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  DefinitionsServerConnectionInitializer definitionsServerConnectionInitializer;
  try {
    definitionsServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  ServiceLocatorClientConfig serviceLocatorClientConfig;
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'service_locator': " << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  ApplicationServiceLocatorClient serviceLocatorClient;
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_address,
      Ref(socketThreadPool), Ref(timerThreadPool));
    serviceLocatorClient->SetCredentials(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password);
    serviceLocatorClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error logging in: " << e.what() << endl;
    return -1;
  }
  vector<ComplianceRuleSchema> complianceRuleSchemas;
  complianceRuleSchemas.push_back(BuildBuyingPowerComplianceRuleSchema());
  complianceRuleSchemas.push_back(
    BuildCancelRestrictionPeriodComplianceRuleSchema());
  complianceRuleSchemas.push_back(
    BuildOpposingOrderCancellationComplianceRuleSchema());
  complianceRuleSchemas.push_back(
    BuildOpposingOrderSubmissionComplianceRuleSchema());
  complianceRuleSchemas.push_back(BuildOrderCountPerSideComplianceRuleSchema());
  complianceRuleSchemas.push_back(
    BuildSubmissionRestrictionPeriodComplianceRuleSchema());
  complianceRuleSchemas.push_back(BuildSymbolRestrictionComplianceRuleSchema());
  boost::optional<DefinitionsServletContainer> definitionsServer;
  try {
    auto minimumSpireClientVersion = Extract<string>(config,
      "minimum_spire_version");
    auto organizationName = Extract<string>(config, "organization",
      "Spire Trading Inc.");
    ifstream timeZoneDatabaseFile{Extract<string>(config, "time_zones")};
    if(!timeZoneDatabaseFile.good()) {
      cerr << "Error parsing time zones." << endl;
      return -1;
    }
    stringstream timeZoneDatabaseBuffer;
    timeZoneDatabaseBuffer << timeZoneDatabaseFile.rdbuf();
    auto countriesConfigFile = Extract<string>(config, "countries",
      "countries.yml");
    auto countriesConfig = Require(LoadFile, countriesConfigFile);
    auto countryDatabase = ParseCountryDatabase(GetNode(countriesConfig,
      "countries"));
    auto currenciesConfigFile = Extract<string>(config, "currencies",
      "currencies.yml");
    auto currenciesConfig = Require(LoadFile, currenciesConfigFile);
    auto currencyDatabase = ParseCurrencyDatabase(GetNode(currenciesConfig,
      "currencies"));
    auto marketsConfigFile = Extract<string>(config, "markets", "markets.yml");
    auto marketsConfig = Require(LoadFile, marketsConfigFile);
    auto marketDatabase = ParseMarketDatabase(GetNode(marketsConfig, "markets"),
      countryDatabase, currencyDatabase);
    auto destinationsConfigFile = Extract<string>(config, "destinations",
      "destinations.yml");
    auto destinationsConfig = Require(LoadFile, destinationsConfigFile);
    auto destinationDatabase = ParseDestinationDatabase(
      destinationsConfig, marketDatabase);
    auto exchangeRates = ParseExchangeRates(GetNode(config, "exchange_rates"));
    definitionsServer.emplace(Initialize(serviceLocatorClient.Get(),
      Initialize(minimumSpireClientVersion, organizationName,
        timeZoneDatabaseBuffer.str(), std::move(countryDatabase),
        std::move(currencyDatabase), std::move(marketDatabase),
        std::move(destinationDatabase), std::move(exchangeRates),
        std::move(complianceRuleSchemas))),
      Initialize(definitionsServerConnectionInitializer.m_interface,
      Ref(socketThreadPool)),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10),
      Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    cerr << "Error initializing definitions server: " << e.what() << endl;
    return -1;
  }
  try {
    definitionsServer->Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    auto ntpPool = Extract<vector<IpAddress>>(config, "ntp_pool");
    JsonObject ntpService;
    ntpService["addresses"] = lexical_cast<std::string>(Stream(ntpPool));
    serviceLocatorClient->Register(TimeService::SERVICE_NAME, ntpService);
    JsonObject definitionsService;
    definitionsService["addresses"] = lexical_cast<std::string>(
      Stream(definitionsServerConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(
      definitionsServerConnectionInitializer.m_serviceName, definitionsService);
  } catch(const std::exception& e) {
    cerr << "Error registering service: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
