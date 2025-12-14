#ifndef NEXUS_DEFINITIONS_CLIENT_HPP
#define NEXUS_DEFINITIONS_CLIENT_HPP
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/Definitions/Venue.hpp"

namespace Nexus {

  /** Concept for types that can be used as a DefinitionsClient. */
  template<typename T>
  concept IsDefinitionsClient = Beam::IsConnection<T> && requires(T& client) {
    { client.load_minimum_spire_client_version() } ->
      std::same_as<std::string>;
    { client.load_organization_name() } -> std::same_as<std::string>;
    { client.load_country_database() } -> std::same_as<CountryDatabase>;
    { client.load_time_zone_database() } ->
      std::same_as<boost::local_time::tz_database>;
    { client.load_currency_database() } -> std::same_as<CurrencyDatabase>;
    { client.load_destination_database() } ->
      std::same_as<DestinationDatabase>;
    { client.load_venue_database() } -> std::same_as<VenueDatabase>;
    { client.load_exchange_rates() } -> std::same_as<std::vector<ExchangeRate>>;
    { client.load_compliance_rule_schemas() } ->
      std::same_as<std::vector<ComplianceRuleSchema>>;
    { client.load_trading_schedule() } -> std::same_as<TradingSchedule>;
  };

  /** Provides a generic interface over an arbitrary DefinitionsClient. */
  class DefinitionsClient {
    public:

      /**
       * Constructs a DefinitionsClient of a specified type using emplacement.
       * @tparam T The type of definitions client to emplace.
       * @param args The arguments to pass to the emplaced definitions client.
       */
      template<IsDefinitionsClient T, typename... Args>
      explicit DefinitionsClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a DefinitionsClient by referencing an existing client.
       * @param client The client to reference.
       */
      template<Beam::DisableCopy<DefinitionsClient> T> requires
        IsDefinitionsClient<Beam::dereference_t<T>>
      DefinitionsClient(T&& client);

      DefinitionsClient(const DefinitionsClient&) = default;
      DefinitionsClient(DefinitionsClient&&) = default;

      /** Returns the minimum Spire client version. */
      std::string load_minimum_spire_client_version();

      /** Returns the organization name. */
      std::string load_organization_name();

      /** Returns the country database. */
      CountryDatabase load_country_database();

      /** Returns the time zone database. */
      boost::local_time::tz_database load_time_zone_database();

      /** Returns the currency database. */
      CurrencyDatabase load_currency_database();

      /** Returns the destination database. */
      DestinationDatabase load_destination_database();

      /** Returns the venue database. */
      VenueDatabase load_venue_database();

      /** Returns the exchange rates. */
      std::vector<ExchangeRate> load_exchange_rates();

      /** Returns the compliance rule schemas. */
      std::vector<ComplianceRuleSchema> load_compliance_rule_schemas();

      /** Returns the trading schedule. */
      TradingSchedule load_trading_schedule();

      void close();

    private:
      struct VirtualDefinitionsClient {
        virtual ~VirtualDefinitionsClient() = default;

        virtual std::string load_minimum_spire_client_version() = 0;
        virtual std::string load_organization_name() = 0;
        virtual CountryDatabase load_country_database() = 0;
        virtual boost::local_time::tz_database load_time_zone_database() = 0;
        virtual CurrencyDatabase load_currency_database() = 0;
        virtual DestinationDatabase load_destination_database() = 0;
        virtual VenueDatabase load_venue_database() = 0;
        virtual std::vector<ExchangeRate> load_exchange_rates() = 0;
        virtual std::vector<ComplianceRuleSchema>
          load_compliance_rule_schemas() = 0;
        virtual TradingSchedule load_trading_schedule() = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedDefinitionsClient final : VirtualDefinitionsClient {
        using DefinitionsClient = C;
        Beam::local_ptr_t<DefinitionsClient> m_client;

        template<typename... Args>
        WrappedDefinitionsClient(Args&&... args);

        std::string load_minimum_spire_client_version() override;
        std::string load_organization_name() override;
        CountryDatabase load_country_database() override;
        boost::local_time::tz_database load_time_zone_database() override;
        CurrencyDatabase load_currency_database() override;
        DestinationDatabase load_destination_database() override;
        VenueDatabase load_venue_database() override;
        std::vector<ExchangeRate> load_exchange_rates() override;
        std::vector<ComplianceRuleSchema>
          load_compliance_rule_schemas() override;
        TradingSchedule load_trading_schedule() override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualDefinitionsClient> m_client;
  };

  template<IsDefinitionsClient T, typename... Args>
  DefinitionsClient::DefinitionsClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(Beam::make_virtual_ptr<WrappedDefinitionsClient<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<DefinitionsClient> T> requires
    IsDefinitionsClient<Beam::dereference_t<T>>
  DefinitionsClient::DefinitionsClient(T&& client)
    : m_client(Beam::make_virtual_ptr<
        WrappedDefinitionsClient<std::remove_cvref_t<T>>>(
          std::forward<T>(client))) {}

  inline std::string DefinitionsClient::load_minimum_spire_client_version() {
    return m_client->load_minimum_spire_client_version();
  }

  inline std::string DefinitionsClient::load_organization_name() {
    return m_client->load_organization_name();
  }

  inline CountryDatabase DefinitionsClient::load_country_database() {
    return m_client->load_country_database();
  }

  inline boost::local_time::tz_database
      DefinitionsClient::load_time_zone_database() {
    return m_client->load_time_zone_database();
  }

  inline CurrencyDatabase DefinitionsClient::load_currency_database() {
    return m_client->load_currency_database();
  }

  inline DestinationDatabase DefinitionsClient::load_destination_database() {
    return m_client->load_destination_database();
  }

  inline VenueDatabase DefinitionsClient::load_venue_database() {
    return m_client->load_venue_database();
  }

  inline std::vector<ExchangeRate> DefinitionsClient::load_exchange_rates() {
    return m_client->load_exchange_rates();
  }

  inline std::vector<ComplianceRuleSchema>
      DefinitionsClient::load_compliance_rule_schemas() {
    return m_client->load_compliance_rule_schemas();
  }

  inline TradingSchedule DefinitionsClient::load_trading_schedule() {
    return m_client->load_trading_schedule();
  }

  inline void DefinitionsClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  DefinitionsClient::WrappedDefinitionsClient<C>::WrappedDefinitionsClient(
    Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::string DefinitionsClient::WrappedDefinitionsClient<C>::
      load_minimum_spire_client_version() {
    return m_client->load_minimum_spire_client_version();
  }

  template<typename C>
  std::string DefinitionsClient::WrappedDefinitionsClient<C>::
      load_organization_name() {
    return m_client->load_organization_name();
  }

  template<typename C>
  CountryDatabase DefinitionsClient::WrappedDefinitionsClient<C>::
      load_country_database() {
    return m_client->load_country_database();
  }

  template<typename C>
  boost::local_time::tz_database DefinitionsClient::
      WrappedDefinitionsClient<C>::load_time_zone_database() {
    return m_client->load_time_zone_database();
  }

  template<typename C>
  CurrencyDatabase DefinitionsClient::WrappedDefinitionsClient<C>::
      load_currency_database() {
    return m_client->load_currency_database();
  }

  template<typename C>
  DestinationDatabase DefinitionsClient::WrappedDefinitionsClient<C>::
      load_destination_database() {
    return m_client->load_destination_database();
  }

  template<typename C>
  VenueDatabase DefinitionsClient::WrappedDefinitionsClient<C>::
      load_venue_database() {
    return m_client->load_venue_database();
  }

  template<typename C>
  std::vector<ExchangeRate> DefinitionsClient::WrappedDefinitionsClient<C>::
      load_exchange_rates() {
    return m_client->load_exchange_rates();
  }

  template<typename C>
  std::vector<ComplianceRuleSchema>
      DefinitionsClient::WrappedDefinitionsClient<C>::
        load_compliance_rule_schemas() {
    return m_client->load_compliance_rule_schemas();
  }

  template<typename C>
  TradingSchedule DefinitionsClient::WrappedDefinitionsClient<C>::
      load_trading_schedule() {
    return m_client->load_trading_schedule();
  }

  template<typename C>
  void DefinitionsClient::WrappedDefinitionsClient<C>::close() {
    m_client->close();
  }
}

#endif
