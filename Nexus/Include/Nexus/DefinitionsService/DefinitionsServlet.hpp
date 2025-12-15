#ifndef NEXUS_DEFINITIONS_SERVLET_HPP
#define NEXUS_DEFINITIONS_SERVLET_HPP
#include "Nexus/DefinitionsService/DefinitionsServices.hpp"
#include "Nexus/DefinitionsService/DefinitionsSession.hpp"

namespace Nexus {

  /**
   * Provides system wide definitions.
   * @param <C> The container instantiating this servlet.
   */
  template<typename C>
  class DefinitionsServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a DefinitionsServlet.
       * @param minimum_spire_client_version The minimum version of the Spire
       *        client required to login.
       * @param organization_name The name of the organization.
       * @param time_zone_database The time zone database.
       * @param country_database The CountryDatabase to disseminate.
       * @param currency_database The CurrencyDatabase to disseminate.
       * @param destination_database The DestinationDatabase to disseminate.
       * @param venue_database The MarketDatabase to disseminate.
       * @param exchange_rates The list of ExchangeRates.
       * @param compliance_rule_schemas The list of ComplianceRuleSchemas.
       * @param trading_schedule The TradingSchedule to disseminate.
       */
      DefinitionsServlet(std::string minimum_spire_client_version,
        std::string organization_name, std::string time_zone_database,
        CountryDatabase country_database, CurrencyDatabase currency_database,
        DestinationDatabase destination_database, VenueDatabase venue_database,
        std::vector<ExchangeRate> exchange_rates,
        std::vector<ComplianceRuleSchema> compliance_rule_schemas,
        TradingSchedule trading_schedule);

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void close();

    private:
      std::string m_minimum_spire_client_version;
      std::string m_organization_name;
      std::string m_time_zone_database;
      CountryDatabase m_country_database;
      CurrencyDatabase m_currency_database;
      DestinationDatabase m_destination_database;
      VenueDatabase m_venue_database;
      std::vector<ExchangeRate> m_exchange_rates;
      std::vector<ComplianceRuleSchema> m_compliance_rule_schemas;
      TradingSchedule m_trading_schedule;
      Beam::OpenState m_open_state;

      std::string on_load_minimum_spire_client_version(
        ServiceProtocolClient& client);
      std::string on_load_organization_name(ServiceProtocolClient& client);
      CountryDatabase on_load_country_database(ServiceProtocolClient& client);
      std::string on_load_time_zone_database(ServiceProtocolClient& client);
      CurrencyDatabase on_load_currency_database(ServiceProtocolClient& client);
      DestinationDatabase on_load_destination_database(
        ServiceProtocolClient& client);
      VenueDatabase on_load_venue_database(ServiceProtocolClient& client);
      std::vector<ExchangeRate> on_load_exchange_rates(
        ServiceProtocolClient& client);
      std::vector<ComplianceRuleSchema> on_load_compliance_rule_schemas(
        ServiceProtocolClient& client);
      TradingSchedule on_load_trading_schedule(ServiceProtocolClient& client);
  };

  struct MetaDefinitionsServlet {
    using Session = DefinitionsSession;
    template<typename C>
    struct apply {
      using type = DefinitionsServlet<C>;
    };
  };

  template<typename C>
  DefinitionsServlet<C>::DefinitionsServlet(
    std::string minimum_spire_client_version, std::string organization_name,
    std::string time_zone_database, CountryDatabase country_database,
    CurrencyDatabase currency_database,
    DestinationDatabase destination_database, VenueDatabase venue_database,
    std::vector<ExchangeRate> exchange_rates,
    std::vector<ComplianceRuleSchema> compliance_rule_schemas,
    TradingSchedule trading_schedule)
    : m_minimum_spire_client_version(std::move(minimum_spire_client_version)),
      m_organization_name(std::move(organization_name)),
      m_time_zone_database(std::move(time_zone_database)),
      m_country_database(std::move(country_database)),
      m_currency_database(std::move(currency_database)),
      m_destination_database(std::move(destination_database)),
      m_venue_database(std::move(venue_database)),
      m_exchange_rates(std::move(exchange_rates)),
      m_compliance_rule_schemas(std::move(compliance_rule_schemas)),
      m_trading_schedule(std::move(trading_schedule)) {}

  template<typename C>
  void DefinitionsServlet<C>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    register_definitions_services(out(slots));
    LoadMinimumSpireClientVersionService::add_slot(out(slots), std::bind_front(
      &DefinitionsServlet::on_load_minimum_spire_client_version, this));
    LoadOrganizationNameService::add_slot(out(slots), std::bind_front(
      &DefinitionsServlet::on_load_organization_name, this));
    LoadCountryDatabaseService::add_slot(out(slots),
      std::bind_front(&DefinitionsServlet::on_load_country_database, this));
    LoadTimeZoneDatabaseService::add_slot(out(slots),
      std::bind_front(&DefinitionsServlet::on_load_time_zone_database, this));
    LoadCurrencyDatabaseService::add_slot(out(slots),
      std::bind_front(&DefinitionsServlet::on_load_currency_database, this));
    LoadDestinationDatabaseService::add_slot(out(slots),
      std::bind_front(&DefinitionsServlet::on_load_destination_database, this));
    LoadVenueDatabaseService::add_slot(out(slots),
      std::bind_front(&DefinitionsServlet::on_load_venue_database, this));
    LoadExchangeRatesService::add_slot(out(slots),
      std::bind_front(&DefinitionsServlet::on_load_exchange_rates, this));
    LoadComplianceRuleSchemasService::add_slot(out(slots), std::bind_front(
      &DefinitionsServlet::on_load_compliance_rule_schemas, this));
    LoadTradingScheduleService::add_slot(out(slots),
      std::bind_front(&DefinitionsServlet::on_load_trading_schedule, this));
  }

  template<typename C>
  void DefinitionsServlet<C>::close() {
    m_open_state.close();
  }

  template<typename C>
  std::string DefinitionsServlet<C>::on_load_minimum_spire_client_version(
      ServiceProtocolClient& client) {
    return m_minimum_spire_client_version;
  }

  template<typename C>
  std::string DefinitionsServlet<C>::on_load_organization_name(
      ServiceProtocolClient& client) {
    return m_organization_name;
  }

  template<typename C>
  CountryDatabase DefinitionsServlet<C>::on_load_country_database(
      ServiceProtocolClient& client) {
    return m_country_database;
  }

  template<typename C>
  std::string DefinitionsServlet<C>::on_load_time_zone_database(
      ServiceProtocolClient& client) {
    return m_time_zone_database;
  }

  template<typename C>
  CurrencyDatabase DefinitionsServlet<C>::on_load_currency_database(
      ServiceProtocolClient& client) {
    return m_currency_database;
  }

  template<typename C>
  DestinationDatabase DefinitionsServlet<C>::on_load_destination_database(
      ServiceProtocolClient& client) {
    return m_destination_database;
  }

  template<typename C>
  VenueDatabase DefinitionsServlet<C>::on_load_venue_database(
      ServiceProtocolClient& client) {
    return m_venue_database;
  }

  template<typename C>
  std::vector<ExchangeRate> DefinitionsServlet<C>::on_load_exchange_rates(
      ServiceProtocolClient& client) {
    return m_exchange_rates;
  }

  template<typename C>
  std::vector<ComplianceRuleSchema>
      DefinitionsServlet<C>::on_load_compliance_rule_schemas(
        ServiceProtocolClient& client) {
    return m_compliance_rule_schemas;
  }

  template<typename C>
  TradingSchedule DefinitionsServlet<C>::on_load_trading_schedule(
      ServiceProtocolClient& client) {
    return m_trading_schedule;
  }
}

#endif
