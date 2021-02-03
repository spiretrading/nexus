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

namespace {
  using DefinitionsServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaDefinitionsServlet,
      ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  std::vector<ExchangeRate> ParseExchangeRates(const YAML::Node& config) {
    return TryOrNest([&] {
      auto exchangeRates = std::vector<ExchangeRate>();
      for(auto& entry : config) {
        auto symbol = Extract<std::string>(entry, "symbol");
        auto pair = ParseCurrencyPair(symbol);
        auto rate = Extract<rational<int>>(entry, "rate");
        auto exchangeRate = ExchangeRate(pair, rate);
        exchangeRates.push_back(exchangeRate);
      }
      return exchangeRates;
    }, std::runtime_error("Failed to parse exchange rates."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "1.0-r"
      DEFINITIONS_SERVER_VERSION "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "server"),
        DefinitionsService::SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto complianceRuleSchemas = std::vector<ComplianceRuleSchema>();
    complianceRuleSchemas.push_back(MakeBuyingPowerComplianceRuleSchema());
    complianceRuleSchemas.push_back(
      MakeCancelRestrictionPeriodComplianceRuleSchema());
    complianceRuleSchemas.push_back(
      MakeOpposingOrderCancellationComplianceRuleSchema());
    complianceRuleSchemas.push_back(
      MakeOpposingOrderSubmissionComplianceRuleSchema());
    complianceRuleSchemas.push_back(
      MakeOrderCountPerSideComplianceRuleSchema());
    complianceRuleSchemas.push_back(
      MakeSubmissionRestrictionPeriodComplianceRuleSchema());
    complianceRuleSchemas.push_back(
      MakeSymbolRestrictionComplianceRuleSchema());
    auto minimumSpireClientVersion = Extract<std::string>(config,
      "minimum_spire_version");
    auto organizationName = Extract<std::string>(config, "organization",
      "Spire Trading Inc.");
    auto timeZoneDatabase = [&] {
      auto file = std::ifstream(Extract<std::string>(config, "time_zones"));
      if(!file.good()) {
        throw std::runtime_error("Time zone database file not found.");
      }
      auto stream = std::stringstream();
      stream << file.rdbuf();
      return stream.str();
    }();
    auto countryDatabase = ParseCountryDatabase(
      GetNode(Require(LoadFile, Extract<std::string>(config, "countries",
        "countries.yml")), "countries"));
    auto currencyDatabase = ParseCurrencyDatabase(GetNode(
      Require(LoadFile, Extract<std::string>(config, "currencies",
        "currencies.yml")), "currencies"));
    auto marketDatabase = ParseMarketDatabase(GetNode(
      Require(LoadFile, Extract<std::string>(config, "markets", "markets.yml")),
        "markets"), countryDatabase, currencyDatabase);
    auto destinationDatabase = ParseDestinationDatabase(
      Require(LoadFile, Extract<std::string>(config, "destinations",
        "destinations.yml")), marketDatabase);
    auto exchangeRates = ParseExchangeRates(GetNode(config, "exchange_rates"));
    auto definitionsServer = DefinitionsServletContainer(Initialize(
      serviceLocatorClient.Get(), Initialize(
        std::move(minimumSpireClientVersion), std::move(organizationName),
        std::move(timeZoneDatabase), std::move(countryDatabase),
        std::move(currencyDatabase), std::move(marketDatabase),
        std::move(destinationDatabase), std::move(exchangeRates),
        std::move(complianceRuleSchemas))),
      Initialize(serviceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    TryOrNest([&] {
      auto ntpPool = Extract<std::vector<IpAddress>>(config, "ntp_pool");
      auto ntpService = JsonObject();
      ntpService["addresses"] = lexical_cast<std::string>(Stream(ntpPool));
      serviceLocatorClient->Register(TimeService::SERVICE_NAME, ntpService);
    }, std::runtime_error("Error registering NTP services."));
    Register(*serviceLocatorClient, serviceConfig);
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
