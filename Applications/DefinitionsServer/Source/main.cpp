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
#include "Nexus/Compliance/OpposingCancelComplianceRule.hpp"
#include "Nexus/Compliance/OpposingSubmissionComplianceRule.hpp"
#include "Nexus/Compliance/OrderCountLimitComplianceRule.hpp"
#include "Nexus/Compliance/RegionFilterComplianceRule.hpp"
#include "Nexus/Compliance/RejectCancelsComplianceRule.hpp"
#include "Nexus/Compliance/RejectSubmissionsComplianceRule.hpp"
#include "Nexus/Compliance/TimeFilterComplianceRule.hpp"
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

namespace {
  using DefinitionsServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaDefinitionsServlet,
      ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  std::vector<ExchangeRate> parse_exchange_rates(const YAML::Node& config) {
    return TryOrNest([&] {
      auto rates = std::vector<ExchangeRate>();
      for(auto& entry : config) {
        auto symbol = Extract<std::string>(entry, "symbol");
        auto pair = parse_currency_pair(symbol);
        auto rate = Extract<rational<int>>(entry, "rate");
        rates.push_back(ExchangeRate(pair, rate));
      }
      return rates;
    }, std::runtime_error("Failed to parse exchange rates."));
  }

  std::vector<ComplianceRuleSchema> make_compliance_rule_schemas() {
    auto schemas = std::vector<ComplianceRuleSchema>();
    schemas.push_back(make_region_filter_compliance_rule_schema(
      make_time_filter_compliance_rule_schema(
        make_reject_cancels_compliance_rule_schema())));
    schemas.push_back(make_region_filter_compliance_rule_schema(
      make_time_filter_compliance_rule_schema(
        make_reject_submissions_compliance_rule_schema())));
    schemas.push_back(make_region_filter_compliance_rule_schema(
      make_buying_power_compliance_rule_schema()));
    schemas.push_back(make_region_filter_compliance_rule_schema(
      make_time_filter_compliance_rule_schema(
        make_per_security_compliance_rule_schema(
          make_opposing_cancel_compliance_rule_schema()))));
    schemas.push_back(make_region_filter_compliance_rule_schema(
      make_time_filter_compliance_rule_schema(
        make_per_security_compliance_rule_schema(
          make_opposing_submission_compliance_rule_schema()))));
    schemas.push_back(make_region_filter_compliance_rule_schema(
      make_per_security_compliance_rule_schema(
        make_order_count_limit_per_side_compliance_rule_schema())));
    return schemas;
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "1.0-r"
      DEFINITIONS_SERVER_VERSION "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "server"), DEFINITIONS_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto minimum_client_version =
      Extract<std::string>(config, "minimum_spire_version");
    auto organization_name =
      Extract<std::string>(config, "organization", "Spire Trading Inc.");
    auto time_zone_database = [&] {
      auto file = std::ifstream(Extract<std::string>(config, "time_zones"));
      if(!file.good()) {
        throw std::runtime_error("Time zone database file not found.");
      }
      auto stream = std::stringstream();
      stream << file.rdbuf();
      return stream.str();
    }();
    auto countries = parse_country_database(
      GetNode(Require(LoadFile, Extract<std::string>(
        config, "countries", "countries.yml")), "countries"));
    auto currencies = parse_currency_database(
      GetNode(Require(LoadFile, Extract<std::string>(
        config, "currencies", "currencies.yml")), "currencies"));
    auto venues = parse_venue_database(GetNode(
      Require(LoadFile, Extract<std::string>(config, "venues", "venues.yml")),
      "venues"), countries, currencies);
    auto destinations = parse_destination_database(
      Require(LoadFile, Extract<std::string>(
        config, "destinations", "destinations.yml")), venues);
    auto rates = parse_exchange_rates(GetNode(config, "exchange_rates"));
    auto schemas = make_compliance_rule_schemas();
    auto schedule = TradingSchedule();
    auto definitions_server = DefinitionsServletContainer(
      Initialize(service_locator_client.Get(), Initialize(
        std::move(minimum_client_version), std::move(organization_name),
        std::move(time_zone_database), std::move(countries),
        std::move(currencies), std::move(destinations), std::move(venues),
        std::move(rates), std::move(schemas), schedule)),
      Initialize(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    TryOrNest([&] {
      auto ntp_pool = Extract<std::vector<IpAddress>>(config, "ntp_pool");
      auto ntp_service = JsonObject();
      ntp_service["addresses"] = lexical_cast<std::string>(Stream(ntp_pool));
      service_locator_client->Register(TimeService::SERVICE_NAME, ntp_service);
    }, std::runtime_error("Error registering NTP services."));
    Register(*service_locator_client, service_config);
    WaitForKillEvent();
    service_locator_client->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
