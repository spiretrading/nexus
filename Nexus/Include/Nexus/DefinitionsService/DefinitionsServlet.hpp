#ifndef NEXUS_DEFINITIONS_SERVLET_HPP
#define NEXUS_DEFINITIONS_SERVLET_HPP
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"
#include "Nexus/DefinitionsService/DefinitionsServices.hpp"
#include "Nexus/DefinitionsService/DefinitionsSession.hpp"

namespace Nexus::DefinitionsService {

  /**
   * Provides system wide definitions.
   * @param <C> The container instantiating this servlet.
   */
  template<typename C>
  class DefinitionsServlet : private boost::noncopyable {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a DefinitionsServlet.
       * @param minimumSpireClientVersion The minimum version of the Spire
       *        client required to login.
       * @param organizationName The name of the organization.
       * @param timeZoneDatabase The time zone database.
       * @param countryDatabase The CountryDatabase to disseminate.
       * @param currencyDatabase The CurrencyDatabase to disseminate.
       * @param marketDatabase The MarketDatabase to disseminate.
       * @param destinationDatabase The DestinationDatabase to disseminate.
       * @param exchangeRates The list of ExchangeRates.
       * @param complianceRuleSchemas The list of ComplianceRuleSchemas.
       */
      DefinitionsServlet(std::string minimumSpireClientVersion,
        std::string organizationName, std::string timeZoneDatabase,
        CountryDatabase countryDatabase, CurrencyDatabase currencyDatabase,
        MarketDatabase marketDatabase, DestinationDatabase destinationDatabase,
        std::vector<ExchangeRate> exchangeRates,
        std::vector<Compliance::ComplianceRuleSchema> complianceRuleSchemas);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void Open();

      void Close();

    private:
      std::string m_minimumSpireClientVersion;
      std::string m_organizationName;
      std::string m_timeZoneDatabase;
      CountryDatabase m_countryDatabase;
      CurrencyDatabase m_currencyDatabase;
      DestinationDatabase m_destinationDatabase;
      MarketDatabase m_marketDatabase;
      std::vector<ExchangeRate> m_exchangeRates;
      std::vector<Compliance::ComplianceRuleSchema> m_complianceRuleSchemas;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      std::string OnLoadMinimumSpireClientVersion(
        ServiceProtocolClient& client);
      std::string OnLoadOrganizationName(ServiceProtocolClient& client);
      CountryDatabase OnLoadCountryDatabase(ServiceProtocolClient& client);
      std::string OnLoadTimeZoneDatabase(ServiceProtocolClient& client);
      CurrencyDatabase OnLoadCurrencyDatabase(ServiceProtocolClient& client);
      DestinationDatabase OnLoadDestinationDatabase(
        ServiceProtocolClient& client);
      MarketDatabase OnLoadMarketDatabase(ServiceProtocolClient& client);
      std::vector<ExchangeRate> OnLoadExchangeRates(
        ServiceProtocolClient& client);
      std::vector<Compliance::ComplianceRuleSchema> OnLoadComplianceRuleSchemas(
        ServiceProtocolClient& client);
      TradingSchedule OnLoadTradingSchedule(ServiceProtocolClient& client);
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
    std::string minimumSpireClientVersion, std::string organizationName,
    std::string timeZoneDatabase, CountryDatabase countryDatabase,
    CurrencyDatabase currencyDatabase, MarketDatabase marketDatabase,
    DestinationDatabase destinationDatabase,
    std::vector<ExchangeRate> exchangeRates,
    std::vector<Compliance::ComplianceRuleSchema> complianceRuleSchemas)
    : m_minimumSpireClientVersion(std::move(minimumSpireClientVersion)),
      m_organizationName(std::move(organizationName)),
      m_timeZoneDatabase(std::move(timeZoneDatabase)),
      m_countryDatabase(std::move(countryDatabase)),
      m_currencyDatabase(std::move(currencyDatabase)),
      m_marketDatabase(std::move(marketDatabase)),
      m_destinationDatabase(std::move(destinationDatabase)),
      m_exchangeRates(std::move(exchangeRates)),
      m_complianceRuleSchemas(std::move(complianceRuleSchemas)) {}

  template<typename C>
  void DefinitionsServlet<C>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterDefinitionsServices(Store(slots));
    LoadMinimumSpireClientVersionService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadMinimumSpireClientVersion, this,
      std::placeholders::_1));
    LoadOrganizationNameService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadOrganizationName, this,
      std::placeholders::_1));
    LoadCountryDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadCountryDatabase, this, std::placeholders::_1));
    LoadTimeZoneDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadTimeZoneDatabase, this,
      std::placeholders::_1));
    LoadCurrencyDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadCurrencyDatabase, this,
      std::placeholders::_1));
    LoadDestinationDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadDestinationDatabase, this,
      std::placeholders::_1));
    LoadMarketDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadMarketDatabase, this, std::placeholders::_1));
    LoadExchangeRatesService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadExchangeRates, this, std::placeholders::_1));
    LoadComplianceRuleSchemasService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadComplianceRuleSchemas, this,
      std::placeholders::_1));
    LoadTradingScheduleService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadTradingSchedule, this, std::placeholders::_1));
  }

  template<typename C>
  void DefinitionsServlet<C>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  template<typename C>
  void DefinitionsServlet<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C>
  void DefinitionsServlet<C>::Shutdown() {
    m_openState.SetClosed();
  }

  template<typename C>
  std::string DefinitionsServlet<C>::OnLoadMinimumSpireClientVersion(
      ServiceProtocolClient& client) {
    return m_minimumSpireClientVersion;
  }

  template<typename C>
  std::string DefinitionsServlet<C>::OnLoadOrganizationName(
      ServiceProtocolClient& client) {
    return m_organizationName;
  }

  template<typename C>
  CountryDatabase DefinitionsServlet<C>::OnLoadCountryDatabase(
      ServiceProtocolClient& client) {
    return m_countryDatabase;
  }

  template<typename C>
  std::string DefinitionsServlet<C>::OnLoadTimeZoneDatabase(
      ServiceProtocolClient& client) {
    return m_timeZoneDatabase;
  }

  template<typename C>
  CurrencyDatabase DefinitionsServlet<C>::OnLoadCurrencyDatabase(
      ServiceProtocolClient& client) {
    return m_currencyDatabase;
  }

  template<typename C>
  DestinationDatabase DefinitionsServlet<C>::OnLoadDestinationDatabase(
      ServiceProtocolClient& client) {
    return m_destinationDatabase;
  }

  template<typename C>
  MarketDatabase DefinitionsServlet<C>::OnLoadMarketDatabase(
      ServiceProtocolClient& client) {
    return m_marketDatabase;
  }

  template<typename C>
  std::vector<ExchangeRate> DefinitionsServlet<C>::OnLoadExchangeRates(
      ServiceProtocolClient& client) {
    return m_exchangeRates;
  }

  template<typename C>
  std::vector<Compliance::ComplianceRuleSchema>
      DefinitionsServlet<C>::OnLoadComplianceRuleSchemas(
      ServiceProtocolClient& client) {
    return m_complianceRuleSchemas;
  }

  template<typename C>
  TradingSchedule DefinitionsServlet<C>::OnLoadTradingSchedule(
      ServiceProtocolClient& client) {
    return TradingSchedule();
  }
}

#endif
