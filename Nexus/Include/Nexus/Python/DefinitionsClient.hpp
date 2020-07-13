#ifndef NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#define NEXUS_PYTHON_DEFINITIONS_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"

namespace Nexus::DefinitionsService {

  /**
   * Wraps a DefinitionsClient for use with Python.
   * @param <C> The type of DefinitionsClient to wrap.
   */
  template<typename C>
  class ToPythonDefinitionsClient final : public VirtualDefinitionsClient {
    public:

      /** The type of DefinitionsClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonDefinitionsClient.
       * @param client The DefinitionsClient to wrap.
       */
      ToPythonDefinitionsClient(std::unique_ptr<Client> client);

      ~ToPythonDefinitionsClient() override;

      std::string LoadMinimumSpireClientVersion() override;

      std::string LoadOrganizationName() override;

      CountryDatabase LoadCountryDatabase() override;

      boost::local_time::tz_database LoadTimeZoneDatabase() override;

      CurrencyDatabase LoadCurrencyDatabase() override;

      DestinationDatabase LoadDestinationDatabase() override;

      MarketDatabase LoadMarketDatabase() override;

      std::vector<ExchangeRate> LoadExchangeRates() override;

      std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas() override;

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonDefinitionsClient.
   * @param client The DefinitionsClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonDefinitionsClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonDefinitionsClient<Client>>(
      std::move(client));
  }

  template<typename C>
  ToPythonDefinitionsClient<C>::ToPythonDefinitionsClient(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonDefinitionsClient<C>::~ToPythonDefinitionsClient() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_client.reset();
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
  void ToPythonDefinitionsClient<C>::Open() {
    auto release = Beam::Python::GilRelease();
    m_client->Open();
  }

  template<typename C>
  void ToPythonDefinitionsClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
