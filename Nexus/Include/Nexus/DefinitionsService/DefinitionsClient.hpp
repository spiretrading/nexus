#ifndef NEXUS_DEFINITIONSCLIENT_HPP
#define NEXUS_DEFINITIONSCLIENT_HPP
#include <sstream>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/DefinitionsService/DefinitionsService.hpp"
#include "Nexus/DefinitionsService/DefinitionsServices.hpp"

namespace Nexus {
namespace DefinitionsService {

  /*! \class DefinitionsClient
      \brief Client used to access the Nexus Definitions service.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class DefinitionsClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<
        ServiceProtocolClientBuilderType>;

      //! Constructs a DefinitionsClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      DefinitionsClient(ClientBuilderForward&& clientBuilder);

      ~DefinitionsClient();

      //! Loads the minimum version of the Spire client required to login.
      std::string LoadMinimumSpireClientVersion();

      //! Loads the CountryDatabase.
      CountryDatabase LoadCountryDatabase();

      //! Loads the time zone database.
      boost::local_time::tz_database LoadTimeZoneDatabase();

      //! Loads the CurrencyDatabase.
      CurrencyDatabase LoadCurrencyDatabase();

      //! Loads the DestinationDatabase.
      DestinationDatabase LoadDestinationDatabase();

      //! Loads the MarketDatabase.
      MarketDatabase LoadMarketDatabase();

      //! Loads the list of ExchangeRates.
      std::vector<ExchangeRate> LoadExchangeRates();

      //! Loads the list of ComplianceRuleSchemas.
      std::vector<Compliance::ComplianceRuleSchema> LoadComplianceRuleSchemas();

      void Open();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  DefinitionsClient<ServiceProtocolClientBuilderType>::DefinitionsClient(
      ClientBuilderForward&& clientBuilder)
      : m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)) {
    RegisterDefinitionsServices(Beam::Store(m_clientHandler.GetSlots()));
  }

  template<typename ServiceProtocolClientBuilderType>
  DefinitionsClient<ServiceProtocolClientBuilderType>::~DefinitionsClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  std::string DefinitionsClient<ServiceProtocolClientBuilderType>::
      LoadMinimumSpireClientVersion() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadMinimumSpireClientVersionService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  CountryDatabase DefinitionsClient<ServiceProtocolClientBuilderType>::
      LoadCountryDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadCountryDatabaseService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  boost::local_time::tz_database DefinitionsClient<
      ServiceProtocolClientBuilderType>::LoadTimeZoneDatabase() {
    auto client = m_clientHandler.GetClient();
    auto timeZones =
      client->template SendRequest<LoadTimeZoneDatabaseService>(0);
    boost::local_time::tz_database database;
    std::stringstream stream(timeZones);
    database.load_from_stream(stream);
    return database;
  }

  template<typename ServiceProtocolClientBuilderType>
  CurrencyDatabase DefinitionsClient<ServiceProtocolClientBuilderType>::
      LoadCurrencyDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadCurrencyDatabaseService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  DestinationDatabase DefinitionsClient<ServiceProtocolClientBuilderType>::
      LoadDestinationDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadDestinationDatabaseService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  MarketDatabase DefinitionsClient<ServiceProtocolClientBuilderType>::
      LoadMarketDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadMarketDatabaseService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<ExchangeRate> DefinitionsClient<
      ServiceProtocolClientBuilderType>::LoadExchangeRates() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadExchangeRatesService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<Compliance::ComplianceRuleSchema>
      DefinitionsClient<ServiceProtocolClientBuilderType>::
      LoadComplianceRuleSchemas() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadComplianceRuleSchemasService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  void DefinitionsClient<ServiceProtocolClientBuilderType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  void DefinitionsClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void DefinitionsClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }
}
}

#endif
