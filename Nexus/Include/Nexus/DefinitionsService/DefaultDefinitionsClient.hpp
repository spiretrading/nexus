#ifndef NEXUS_DEFAULT_DEFINITIONS_CLIENT_HPP
#define NEXUS_DEFAULT_DEFINITIONS_CLIENT_HPP
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"

namespace Nexus {

  /** Implements a DefinitionsClient returning default values. */
  class DefaultDefinitionsClient {
    public:
      std::string load_minimum_spire_client_version();
      std::string load_organization_name();
      CountryDatabase load_country_database();
      boost::local_time::tz_database load_time_zone_database();
      CurrencyDatabase load_currency_database();
      DestinationDatabase load_destination_database();
      VenueDatabase load_venue_database();
      std::vector<ExchangeRate> load_exchange_rates();
      std::vector<ComplianceRuleSchema> load_compliance_rule_schemas();
      TradingSchedule load_trading_schedule();
      void close();
  };

  inline std::string
      DefaultDefinitionsClient::load_minimum_spire_client_version() {
    return "1";
  }

  inline std::string DefaultDefinitionsClient::load_organization_name() {
    return "Spire Trading Inc.";
  }

  inline CountryDatabase DefaultDefinitionsClient::load_country_database() {
    return DEFAULT_COUNTRIES;
  }

  inline boost::local_time::tz_database
      DefaultDefinitionsClient::load_time_zone_database() {
    return get_default_time_zone_database();
  }

  inline CurrencyDatabase DefaultDefinitionsClient::load_currency_database() {
    return DEFAULT_CURRENCIES;
  }

  inline DestinationDatabase
      DefaultDefinitionsClient::load_destination_database() {
    return DEFAULT_DESTINATIONS;
  }

  inline VenueDatabase DefaultDefinitionsClient::load_venue_database() {
    return DEFAULT_VENUES;
  }

  inline std::vector<ExchangeRate>
      DefaultDefinitionsClient::load_exchange_rates() {
    return {};
  }

  inline std::vector<ComplianceRuleSchema>
      DefaultDefinitionsClient::load_compliance_rule_schemas() {
    return {};
  }

  inline TradingSchedule DefaultDefinitionsClient::load_trading_schedule() {
    return {};
  }

  void DefaultDefinitionsClient::close() {}
}

#endif
