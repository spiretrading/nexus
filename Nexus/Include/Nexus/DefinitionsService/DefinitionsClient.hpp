#ifndef NEXUS_DEFINITIONS_CLIENT_HPP
#define NEXUS_DEFINITIONS_CLIENT_HPP
#include <sstream>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/DefinitionsService/DefinitionsService.hpp"
#include "Nexus/DefinitionsService/DefinitionsServices.hpp"

namespace Nexus::DefinitionsService {

  /**
   * Client used to access the Nexus Definitions service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class DefinitionsClient : private boost::noncopyable {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a DefinitionsClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit DefinitionsClient(BF&& clientBuilder);

      ~DefinitionsClient();

      /** Loads the minimum version of the Spire client required to login. */
      std::string LoadMinimumSpireClientVersion();

      /** Loads the name of the organization. */
      std::string LoadOrganizationName();

      /** Loads the CountryDatabase. */
      CountryDatabase LoadCountryDatabase();

      /** Loads the time zone database. */
      boost::local_time::tz_database LoadTimeZoneDatabase();

      /** Loads the CurrencyDatabase. */
      CurrencyDatabase LoadCurrencyDatabase();

      /** Loads the DestinationDatabase. */
      DestinationDatabase LoadDestinationDatabase();

      /** Loads the MarketDatabase. */
      MarketDatabase LoadMarketDatabase();

      /** Loads the list of ExchangeRates. */
      std::vector<ExchangeRate> LoadExchangeRates();

      /** Loads the list of ComplianceRuleSchemas. */
      std::vector<Compliance::ComplianceRuleSchema> LoadComplianceRuleSchemas();

      /** Loads the TradingSchedule. */
      TradingSchedule LoadTradingSchedule();

      void Open();

      void Close();

    private:
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename B>
  template<typename BF>
  DefinitionsClient<B>::DefinitionsClient(BF&& clientBuilder)
      : m_clientHandler(std::forward<BF>(clientBuilder)) {
    RegisterDefinitionsServices(Beam::Store(m_clientHandler.GetSlots()));
  }

  template<typename B>
  DefinitionsClient<B>::~DefinitionsClient() {
    Close();
  }

  template<typename B>
  std::string DefinitionsClient<B>::LoadMinimumSpireClientVersion() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadMinimumSpireClientVersionService>();
  }

  template<typename B>
  std::string DefinitionsClient<B>::LoadOrganizationName() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadOrganizationNameService>();
  }

  template<typename B>
  CountryDatabase DefinitionsClient<B>::LoadCountryDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadCountryDatabaseService>();
  }

  template<typename B>
  boost::local_time::tz_database DefinitionsClient<B>::LoadTimeZoneDatabase() {
    auto client = m_clientHandler.GetClient();
    auto timeZones =
      client->template SendRequest<LoadTimeZoneDatabaseService>();
    auto database = boost::local_time::tz_database();
    auto stream = std::stringstream(timeZones);
    database.load_from_stream(stream);
    return database;
  }

  template<typename B>
  CurrencyDatabase DefinitionsClient<B>::LoadCurrencyDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadCurrencyDatabaseService>();
  }

  template<typename B>
  DestinationDatabase DefinitionsClient<B>::LoadDestinationDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadDestinationDatabaseService>();
  }

  template<typename B>
  MarketDatabase DefinitionsClient<B>::LoadMarketDatabase() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadMarketDatabaseService>();
  }

  template<typename B>
  std::vector<ExchangeRate> DefinitionsClient<B>::LoadExchangeRates() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadExchangeRatesService>();
  }

  template<typename B>
  std::vector<Compliance::ComplianceRuleSchema>
      DefinitionsClient<B>::LoadComplianceRuleSchemas() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadComplianceRuleSchemasService>();
  }

  template<typename B>
  TradingSchedule DefinitionsClient<B>::LoadTradingSchedule() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadTradingScheduleService>();
  }

  template<typename B>
  void DefinitionsClient<B>::Open() {
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

  template<typename B>
  void DefinitionsClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename B>
  void DefinitionsClient<B>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }
}

#endif
