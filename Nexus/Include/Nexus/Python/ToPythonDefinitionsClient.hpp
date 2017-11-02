#ifndef NEXUS_TO_PYTHON_DEFINITIONS_CLIENT_HPP
#define NEXUS_TO_PYTHON_DEFINITIONS_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/DefinitionsService/DefinitionsService.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"

namespace Nexus {
namespace DefinitionsService {

  /*! \class ToPythonDefinitionsClient
      \brief Wraps a DefinitionsClient for use with Python.
      \tparam ClientType The type of DefinitionsClient to wrap.
   */
  template<typename ClientType>
  class ToPythonDefinitionsClient : public VirtualDefinitionsClient {
    public:

      //! The type of DefinitionsClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonDefinitionsClient.
      /*!
        \param client The DefinitionsClient to wrap.
      */
      ToPythonDefinitionsClient(std::unique_ptr<Client> client);

      virtual ~ToPythonDefinitionsClient() override final;

      virtual std::string LoadMinimumSpireClientVersion() override final;

      virtual CountryDatabase LoadCountryDatabase() override final;

      virtual boost::local_time::tz_database
        LoadTimeZoneDatabase() override final;

      virtual CurrencyDatabase LoadCurrencyDatabase() override final;

      virtual DestinationDatabase LoadDestinationDatabase() override final;

      virtual MarketDatabase LoadMarketDatabase() override final;

      virtual std::vector<ExchangeRate> LoadExchangeRates() override final;

      virtual std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas() override final;

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
  };

  //! Makes a ToPythonDefinitionsClient.
  /*!
    \param client The DefinitionsClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonDefinitionsClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonDefinitionsClient<Client>>(
      std::move(client));
  }

  template<typename ClientType>
  ToPythonDefinitionsClient<ClientType>::ToPythonDefinitionsClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonDefinitionsClient<ClientType>::~ToPythonDefinitionsClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client.reset();
  }

  template<typename ClientType>
  std::string ToPythonDefinitionsClient<ClientType>::
      LoadMinimumSpireClientVersion() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadMinimumSpireClientVersion();
  }

  template<typename ClientType>
  CountryDatabase ToPythonDefinitionsClient<ClientType>::LoadCountryDatabase() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadCountryDatabase();
  }

  template<typename ClientType>
  boost::local_time::tz_database
      ToPythonDefinitionsClient<ClientType>::LoadTimeZoneDatabase() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadTimeZoneDatabase();
  }

  template<typename ClientType>
  CurrencyDatabase ToPythonDefinitionsClient<ClientType>::
      LoadCurrencyDatabase() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadCurrencyDatabase();
  }

  template<typename ClientType>
  DestinationDatabase ToPythonDefinitionsClient<ClientType>::
      LoadDestinationDatabase() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadDestinationDatabase();
  }

  template<typename ClientType>
  MarketDatabase ToPythonDefinitionsClient<ClientType>::LoadMarketDatabase() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadMarketDatabase();
  }

  template<typename ClientType>
  std::vector<ExchangeRate> ToPythonDefinitionsClient<ClientType>::
      LoadExchangeRates() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadExchangeRates();
  }

  template<typename ClientType>
  std::vector<Compliance::ComplianceRuleSchema>
      ToPythonDefinitionsClient<ClientType>::LoadComplianceRuleSchemas() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadComplianceRuleSchemas();
  }

  template<typename ClientType>
  void ToPythonDefinitionsClient<ClientType>::Open() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Open();
  }

  template<typename ClientType>
  void ToPythonDefinitionsClient<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->Close();
  }
}
}

#endif
