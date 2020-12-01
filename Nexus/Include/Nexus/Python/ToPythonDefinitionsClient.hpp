#ifndef NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#define NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/DefinitionsService/DefinitionsClientBox.hpp"

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
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

      std::string LoadMinimumSpireClientVersion();

      std::string LoadOrganizationName();

      CountryDatabase LoadCountryDatabase();

      boost::local_time::tz_database LoadTimeZoneDatabase();

      CurrencyDatabase LoadCurrencyDatabase();

      DestinationDatabase LoadDestinationDatabase();

      MarketDatabase LoadMarketDatabase();

      std::vector<ExchangeRate> LoadExchangeRates();

      std::vector<Compliance::ComplianceRuleSchema> LoadComplianceRuleSchemas();

      TradingSchedule LoadTradingSchedule();

      void Close();

    private:
      boost::optional<Client> m_client;

      ToPythonDefinitionsClient(const ToPythonDefinitionsClient&) = delete;
      ToPythonDefinitionsClient& operator =(
        const ToPythonDefinitionsClient&) = delete;
  };

  template<typename Client>
  ToPythonDefinitionsClient(Client&&) ->
    ToPythonDefinitionsClient<std::decay_t<Client>>;

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
      ToPythonDefinitionsClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonDefinitionsClient<C>::Client&
      ToPythonDefinitionsClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  std::string ToPythonDefinitionsClient<C>::LoadMinimumSpireClientVersion() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadMinimumSpireClientVersion();
  }

  template<typename C>
  std::string ToPythonDefinitionsClient<C>::LoadOrganizationName() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadOrganizationName();
  }

  template<typename C>
  CountryDatabase ToPythonDefinitionsClient<C>::LoadCountryDatabase() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadCountryDatabase();
  }

  template<typename C>
  boost::local_time::tz_database
      ToPythonDefinitionsClient<C>::LoadTimeZoneDatabase() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadTimeZoneDatabase();
  }

  template<typename C>
  CurrencyDatabase ToPythonDefinitionsClient<C>::LoadCurrencyDatabase() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadCurrencyDatabase();
  }

  template<typename C>
  DestinationDatabase ToPythonDefinitionsClient<C>::LoadDestinationDatabase() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadDestinationDatabase();
  }

  template<typename C>
  MarketDatabase ToPythonDefinitionsClient<C>::LoadMarketDatabase() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadMarketDatabase();
  }

  template<typename C>
  std::vector<ExchangeRate> ToPythonDefinitionsClient<C>::LoadExchangeRates() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadExchangeRates();
  }

  template<typename C>
  std::vector<Compliance::ComplianceRuleSchema>
      ToPythonDefinitionsClient<C>::LoadComplianceRuleSchemas() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadComplianceRuleSchemas();
  }

  template<typename C>
  TradingSchedule ToPythonDefinitionsClient<C>::LoadTradingSchedule() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadTradingSchedule();
  }

  template<typename C>
  void ToPythonDefinitionsClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
