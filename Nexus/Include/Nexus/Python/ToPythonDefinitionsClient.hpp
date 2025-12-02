#ifndef NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#define NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"

namespace Nexus {

  /**
   * Wraps a DefinitionsClient for use with Python.
   * @param <C> The type of DefinitionsClient to wrap.
   */
  template<IsDefinitionsClient C>
  class ToPythonDefinitionsClient {
    public:

      /** The type of DefinitionsClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonDefinitionsClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonDefinitionsClient(Args&&... args);

      ~ToPythonDefinitionsClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      /** Returns a reference to the underlying client. */
      Client& operator *();

      /** Returns a reference to the underlying client. */
      const Client& operator *() const;

      /** Returns a pointer to the underlying client. */
      Client* operator ->();

      /** Returns a pointer to the underlying client. */
      const Client* operator ->() const;

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

    private:
      boost::optional<Client> m_client;

      ToPythonDefinitionsClient(const ToPythonDefinitionsClient&) = delete;
      ToPythonDefinitionsClient& operator =(
        const ToPythonDefinitionsClient&) = delete;
  };

  template<typename Client>
  ToPythonDefinitionsClient(Client&&) ->
    ToPythonDefinitionsClient<std::remove_cvref_t<Client>>;

  template<IsDefinitionsClient C>
  template<typename... Args>
  ToPythonDefinitionsClient<C>::ToPythonDefinitionsClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsDefinitionsClient C>
  ToPythonDefinitionsClient<C>::~ToPythonDefinitionsClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsDefinitionsClient C>
  typename ToPythonDefinitionsClient<C>::Client&
      ToPythonDefinitionsClient<C>::get() {
    return *m_client;
  }

  template<IsDefinitionsClient C>
  const typename ToPythonDefinitionsClient<C>::Client&
      ToPythonDefinitionsClient<C>::get() const {
    return *m_client;
  }

  template<IsDefinitionsClient C>
  typename ToPythonDefinitionsClient<C>::Client&
      ToPythonDefinitionsClient<C>::operator *() {
    return *m_client;
  }

  template<IsDefinitionsClient C>
  const typename ToPythonDefinitionsClient<C>::Client&
      ToPythonDefinitionsClient<C>::operator *() const {
    return *m_client;
  }

  template<IsDefinitionsClient C>
  typename ToPythonDefinitionsClient<C>::Client*
      ToPythonDefinitionsClient<C>::operator ->() {
    return m_client.get_ptr();
  }

  template<IsDefinitionsClient C>
  const typename ToPythonDefinitionsClient<C>::Client*
      ToPythonDefinitionsClient<C>::operator ->() const {
    return m_client.get_ptr();
  }

  template<IsDefinitionsClient C>
  std::string ToPythonDefinitionsClient<C>::
      load_minimum_spire_client_version() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_minimum_spire_client_version();
  }

  template<IsDefinitionsClient C>
  std::string ToPythonDefinitionsClient<C>::load_organization_name() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_organization_name();
  }

  template<IsDefinitionsClient C>
  CountryDatabase ToPythonDefinitionsClient<C>::load_country_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_country_database();
  }

  template<IsDefinitionsClient C>
  boost::local_time::tz_database
      ToPythonDefinitionsClient<C>::load_time_zone_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_time_zone_database();
  }

  template<IsDefinitionsClient C>
  CurrencyDatabase ToPythonDefinitionsClient<C>::load_currency_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_currency_database();
  }

  template<IsDefinitionsClient C>
  DestinationDatabase
      ToPythonDefinitionsClient<C>::load_destination_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_destination_database();
  }

  template<IsDefinitionsClient C>
  VenueDatabase ToPythonDefinitionsClient<C>::load_venue_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_venue_database();
  }

  template<IsDefinitionsClient C>
  std::vector<ExchangeRate>
      ToPythonDefinitionsClient<C>::load_exchange_rates() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_exchange_rates();
  }

  template<IsDefinitionsClient C>
  std::vector<ComplianceRuleSchema>
      ToPythonDefinitionsClient<C>::load_compliance_rule_schemas() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_compliance_rule_schemas();
  }

  template<IsDefinitionsClient C>
  TradingSchedule ToPythonDefinitionsClient<C>::load_trading_schedule() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_trading_schedule();
  }

  template<IsDefinitionsClient C>
  void ToPythonDefinitionsClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
