#ifndef NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#define NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"

namespace Nexus::DefinitionsService {

  /**
   * Wraps a DefinitionsClient for use with Python.
   * @param <C> The type of DefinitionsClient to wrap.
   */
  template<typename C>
  class ToPythonDefinitionsClient {
    public:

      /** The type of DefinitionsClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonDefinitionsClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonDefinitionsClient, Args...>>
      ToPythonDefinitionsClient(Args&&... args);

      ~ToPythonDefinitionsClient();

      /** Returns the wrapped client. */
      const Client& get_client() const;

      /** Returns the wrapped client. */
      Client& get_client();
      std::string load_minimum_spire_client_version();
      std::string load_organization_name();
      CountryDatabase load_country_database();
      boost::local_time::tz_database load_time_zone_database();
      CurrencyDatabase load_currency_database();
      DestinationDatabase load_destination_database();
      VenueDatabase load_venue_database();
      std::vector<ExchangeRate> load_exchange_rates();
      std::vector<Compliance::ComplianceRuleSchema>
        load_compliance_rule_schemas();
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
    ToPythonDefinitionsClient<std::remove_reference_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonDefinitionsClient<C>::ToPythonDefinitionsClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonDefinitionsClient<C>::~ToPythonDefinitionsClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonDefinitionsClient<C>::Client&
      ToPythonDefinitionsClient<C>::get_client() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonDefinitionsClient<C>::Client&
      ToPythonDefinitionsClient<C>::get_client() {
    return *m_client;
  }

  template<typename C>
  std::string ToPythonDefinitionsClient<C>::load_minimum_spire_client_version() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_minimum_spire_client_version();
  }

  template<typename C>
  std::string ToPythonDefinitionsClient<C>::load_organization_name() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_organization_name();
  }

  template<typename C>
  CountryDatabase ToPythonDefinitionsClient<C>::load_country_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_country_database();
  }

  template<typename C>
  boost::local_time::tz_database
      ToPythonDefinitionsClient<C>::load_time_zone_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_time_zone_database();
  }

  template<typename C>
  CurrencyDatabase ToPythonDefinitionsClient<C>::load_currency_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_currency_database();
  }

  template<typename C>
  DestinationDatabase ToPythonDefinitionsClient<C>::load_destination_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_destination_database();
  }

  template<typename C>
  VenueDatabase ToPythonDefinitionsClient<C>::load_venue_database() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_venue_database();
  }

  template<typename C>
  std::vector<ExchangeRate> ToPythonDefinitionsClient<C>::load_exchange_rates() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_exchange_rates();
  }

  template<typename C>
  std::vector<Compliance::ComplianceRuleSchema>
      ToPythonDefinitionsClient<C>::load_compliance_rule_schemas() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_compliance_rule_schemas();
  }

  template<typename C>
  TradingSchedule ToPythonDefinitionsClient<C>::load_trading_schedule() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_trading_schedule();
  }

  template<typename C>
  void ToPythonDefinitionsClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
