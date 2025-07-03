#ifndef NEXUS_DEFINITIONS_CLIENT_HPP
#define NEXUS_DEFINITIONS_CLIENT_HPP
#include <sstream>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/DefinitionsService/DefinitionsService.hpp"
#include "Nexus/DefinitionsService/DefinitionsServices.hpp"

namespace Nexus::DefinitionsService {

  /**
   * Client used to access the Nexus Definitions service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class DefinitionsClient {
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

      void Close();

    private:
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::IO::OpenState m_openState;

      DefinitionsClient(const DefinitionsClient&) = delete;
      DefinitionsClient& operator =(const DefinitionsClient&) = delete;
  };

  template<typename B>
  template<typename BF>
  DefinitionsClient<B>::DefinitionsClient(BF&& clientBuilder)
      try : m_clientHandler(std::forward<BF>(clientBuilder)) {
    RegisterDefinitionsServices(Beam::Store(m_clientHandler.GetSlots()));
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the definitions server."));
  }

  template<typename B>
  DefinitionsClient<B>::~DefinitionsClient() {
    Close();
  }

  template<typename B>
  std::string DefinitionsClient<B>::LoadMinimumSpireClientVersion() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        LoadMinimumSpireClientVersionService>();
    }, "Failed to load minimum Spire client version.");
  }

  template<typename B>
  std::string DefinitionsClient<B>::LoadOrganizationName() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadOrganizationNameService>();
    }, "Failed to load organization name.");
  }

  template<typename B>
  CountryDatabase DefinitionsClient<B>::LoadCountryDatabase() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadCountryDatabaseService>();
    }, "Failed to load country database.");
  }

  template<typename B>
  boost::local_time::tz_database DefinitionsClient<B>::LoadTimeZoneDatabase() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      auto timeZones =
        client->template SendRequest<LoadTimeZoneDatabaseService>();
      auto database = boost::local_time::tz_database();
      auto stream = std::stringstream(timeZones);
      database.load_from_stream(stream);
      return database;
    }, "Failed to load timezone database.");
  }

  template<typename B>
  CurrencyDatabase DefinitionsClient<B>::LoadCurrencyDatabase() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadCurrencyDatabaseService>();
    }, "Failed to load currency database.");
  }

  template<typename B>
  DestinationDatabase DefinitionsClient<B>::LoadDestinationDatabase() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadDestinationDatabaseService>();
    }, "Failed to load destination database.");
  }

  template<typename B>
  MarketDatabase DefinitionsClient<B>::LoadMarketDatabase() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadMarketDatabaseService>();
    }, "Failed to load market database.");
  }

  template<typename B>
  std::vector<ExchangeRate> DefinitionsClient<B>::LoadExchangeRates() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadExchangeRatesService>();
    }, "Failed to load exchange rates.");
  }

  template<typename B>
  std::vector<Compliance::ComplianceRuleSchema>
      DefinitionsClient<B>::LoadComplianceRuleSchemas() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadComplianceRuleSchemasService>();
    }, "Failed to load compliance rule schemas.");
  }

  template<typename B>
  TradingSchedule DefinitionsClient<B>::LoadTradingSchedule() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadTradingScheduleService>();
    }, "Failed to load trading schedule.");
  }

  template<typename B>
  void DefinitionsClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_openState.Close();
  }
}

#endif
