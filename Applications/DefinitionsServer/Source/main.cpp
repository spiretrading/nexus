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
using namespace TCLAP;

namespace {
  using DefinitionsServletContainer =
    ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
    MetaDefinitionsServlet, ApplicationServiceLocatorClient::Client*>,
    TcpServerSocket, BinarySender<SharedBuffer>, NullEncoder,
    std::shared_ptr<LiveTimer>>;

  struct DefinitionsServerConnectionInitializer {
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void DefinitionsServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<std::string>(config, "service",
      DefinitionsService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<std::vector<IpAddress>>(config, "addresses",
      addresses);
  }

  std::vector<ExchangeRate> ParseExchangeRates(const YAML::Node& config) {
    auto exchangeRates = std::vector<ExchangeRate>();
    for(auto& entry : config) {
      auto symbol = Extract<std::string>(entry, "symbol");
      auto pair = ParseCurrencyPair(symbol);
      auto rate = Extract<rational<int>>(entry, "rate");
      auto exchangeRate = ExchangeRate(pair, rate);
      exchangeRates.push_back(exchangeRate);
    }
    return exchangeRates;
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" DEFINITIONS_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto definitionsServerConnectionInitializer =
    DefinitionsServerConnectionInitializer();
  try {
    definitionsServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'service_locator': " << e.what() <<
      std::endl;
    return -1;
  }
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address);
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto complianceRuleSchemas = std::vector<ComplianceRuleSchema>();
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
  auto definitionsServer = optional<DefinitionsServletContainer>();
  try {
    auto minimumSpireClientVersion = Extract<std::string>(config,
      "minimum_spire_version");
    auto organizationName = Extract<std::string>(config, "organization",
      "Spire Trading Inc.");
    auto timeZoneDatabaseFile = std::ifstream(
      Extract<std::string>(config, "time_zones"));
    if(!timeZoneDatabaseFile.good()) {
      std::cerr << "Error parsing time zones." << std::endl;
      return -1;
    }
    auto timeZoneDatabaseBuffer = std::stringstream();
    timeZoneDatabaseBuffer << timeZoneDatabaseFile.rdbuf();
    auto countriesConfigFile = Extract<std::string>(config, "countries",
      "countries.yml");
    auto countriesConfig = Require(LoadFile, countriesConfigFile);
    auto countryDatabase = ParseCountryDatabase(GetNode(countriesConfig,
      "countries"));
    auto currenciesConfigFile = Extract<std::string>(config, "currencies",
      "currencies.yml");
    auto currenciesConfig = Require(LoadFile, currenciesConfigFile);
    auto currencyDatabase = ParseCurrencyDatabase(GetNode(currenciesConfig,
      "currencies"));
    auto marketsConfigFile = Extract<std::string>(config, "markets",
      "markets.yml");
    auto marketsConfig = Require(LoadFile, marketsConfigFile);
    auto marketDatabase = ParseMarketDatabase(GetNode(marketsConfig, "markets"),
      countryDatabase, currencyDatabase);
    auto destinationsConfigFile = Extract<std::string>(config, "destinations",
      "destinations.yml");
    auto destinationsConfig = Require(LoadFile, destinationsConfigFile);
    auto destinationDatabase = ParseDestinationDatabase(
      destinationsConfig, marketDatabase);
    auto exchangeRates = ParseExchangeRates(GetNode(config, "exchange_rates"));
    try {
      definitionsServer.emplace(Initialize(serviceLocatorClient.Get(),
        Initialize(minimumSpireClientVersion, organizationName,
          timeZoneDatabaseBuffer.str(), std::move(countryDatabase),
          std::move(currencyDatabase), std::move(marketDatabase),
          std::move(destinationDatabase), std::move(exchangeRates),
          std::move(complianceRuleSchemas))),
        Initialize(definitionsServerConnectionInitializer.m_interface),
        std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    } catch(const std::exception& e) {
      std::cerr << "Error opening server: " << e.what() << std::endl;
      return -1;
    }
  } catch(const std::exception& e) {
    std::cerr << "Error initializing definitions server: " << e.what() <<
      std::endl;
    return -1;
  }
  try {
    auto ntpPool = Extract<std::vector<IpAddress>>(config, "ntp_pool");
    auto ntpService = JsonObject();
    ntpService["addresses"] = lexical_cast<std::string>(Stream(ntpPool));
    serviceLocatorClient->Register(TimeService::SERVICE_NAME, ntpService);
    auto definitionsService = JsonObject();
    definitionsService["addresses"] = lexical_cast<std::string>(
      Stream(definitionsServerConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(
      definitionsServerConnectionInitializer.m_serviceName, definitionsService);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
