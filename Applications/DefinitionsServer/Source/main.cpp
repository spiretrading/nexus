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
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
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
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using DefinitionsServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaDefinitionsServlet,
      ApplicationServiceLocatorClient*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  std::vector<ExchangeRate> parse_exchange_rates(const YAML::Node& config) {
    return try_or_nest([&] {
      auto rates = std::vector<ExchangeRate>();
      for(auto& entry : config) {
        auto symbol = extract<std::string>(entry, "symbol");
        auto pair = parse_currency_pair(symbol);
        auto rate = extract<rational<int>>(entry, "rate");
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
    auto config = parse_command_line(argc, argv, "1.0-r"
      DEFINITIONS_SERVER_VERSION "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "server"), DEFINITIONS_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto minimum_client_version =
      extract<std::string>(config, "minimum_spire_version");
    auto organization_name =
      extract<std::string>(config, "organization", "Spire Trading Inc.");
    auto time_zone_database = [&] {
      auto file = std::ifstream(extract<std::string>(config, "time_zones"));
      if(!file.good()) {
        throw_with_location(
          std::runtime_error("Time zone database file not found."));
      }
      auto stream = std::stringstream();
      stream << file.rdbuf();
      return stream.str();
    }();
    auto countries = parse_country_database(
      get_node(load_file(extract<std::string>(
        config, "countries", "countries.yml")), "countries"));
    auto currencies = parse_currency_database(get_node(load_file(
      extract<std::string>(config, "currencies", "currencies.yml")),
      "currencies"));
    auto venues = parse_venue_database(get_node(load_file(extract<std::string>(
      config, "venues", "venues.yml")), "venues"), countries, currencies);
    auto destinations = parse_destination_database(load_file(
      extract<std::string>(config, "destinations", "destinations.yml")),
      venues);
    auto rates = parse_exchange_rates(get_node(config, "exchange_rates"));
    auto schemas = make_compliance_rule_schemas();
    auto schedule = TradingSchedule();
    auto definitions_server = DefinitionsServletContainer(
      init(&service_locator_client, init(std::move(minimum_client_version),
        std::move(organization_name), std::move(time_zone_database),
        std::move(countries), std::move(currencies), std::move(destinations),
        std::move(venues), std::move(rates), std::move(schemas), schedule)),
      init(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    try_or_nest([&] {
      auto ntp_pool = extract<std::vector<IpAddress>>(config, "ntp_pool");
      auto ntp_service = JsonObject();
      ntp_service["addresses"] = lexical_cast<std::string>(Stream(ntp_pool));
      service_locator_client.add(TIME_SERVICE_NAME, ntp_service);
    }, std::runtime_error("Error registering NTP services."));
    add(service_locator_client, service_config);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
