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
      std::string OnLoadMinimumSpireClientVersion(ServiceProtocolClient& client,
        int dummy);
      std::string OnLoadOrganizationName(ServiceProtocolClient& client,
        int dummy);
      CountryDatabase OnLoadCountryDatabase(ServiceProtocolClient& client,
        int dummy);
      std::string OnLoadTimeZoneDatabase(ServiceProtocolClient& client,
        int dummy);
      CurrencyDatabase OnLoadCurrencyDatabase(ServiceProtocolClient& client,
        int dummy);
      DestinationDatabase OnLoadDestinationDatabase(
        ServiceProtocolClient& client, int dummy);
      MarketDatabase OnLoadMarketDatabase(ServiceProtocolClient& client,
        int dummy);
      std::vector<ExchangeRate> OnLoadExchangeRates(
        ServiceProtocolClient& client, int dummy);
      std::vector<Compliance::ComplianceRuleSchema> OnLoadComplianceRuleSchemas(
        ServiceProtocolClient& client, int dummy);
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
      std::placeholders::_1, std::placeholders::_2));
    LoadOrganizationNameService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadOrganizationName, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadCountryDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadCountryDatabase, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadTimeZoneDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadTimeZoneDatabase, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadCurrencyDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadCurrencyDatabase, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadDestinationDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadDestinationDatabase, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadMarketDatabaseService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadMarketDatabase, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadExchangeRatesService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadExchangeRates, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadComplianceRuleSchemasService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadComplianceRuleSchemas, this,
      std::placeholders::_1, std::placeholders::_2));
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
      ServiceProtocolClient& client, int dummy) {
    return m_minimumSpireClientVersion;
  }

  template<typename C>
  std::string DefinitionsServlet<C>::OnLoadOrganizationName(
      ServiceProtocolClient& client, int dummy) {
    return m_organizationName;
  }

  template<typename C>
  CountryDatabase DefinitionsServlet<C>::OnLoadCountryDatabase(
      ServiceProtocolClient& client, int dummy) {
    return m_countryDatabase;
  }

  template<typename C>
  std::string DefinitionsServlet<C>::OnLoadTimeZoneDatabase(
      ServiceProtocolClient& client, int dummy) {
    return m_timeZoneDatabase;
  }

  template<typename C>
  CurrencyDatabase DefinitionsServlet<C>::OnLoadCurrencyDatabase(
      ServiceProtocolClient& client, int dummy) {
    return m_currencyDatabase;
  }

  template<typename C>
  DestinationDatabase DefinitionsServlet<C>::OnLoadDestinationDatabase(
      ServiceProtocolClient& client, int dummy) {
    return m_destinationDatabase;
  }

  template<typename C>
  MarketDatabase DefinitionsServlet<C>::OnLoadMarketDatabase(
      ServiceProtocolClient& client, int dummy) {
    return m_marketDatabase;
  }

  template<typename C>
  std::vector<ExchangeRate> DefinitionsServlet<C>::OnLoadExchangeRates(
      ServiceProtocolClient& client, int dummy) {
    return m_exchangeRates;
  }

  template<typename C>
  std::vector<Compliance::ComplianceRuleSchema>
      DefinitionsServlet<C>::OnLoadComplianceRuleSchemas(
      ServiceProtocolClient& client, int dummy) {
    return m_complianceRuleSchemas;
  }
}

#endif
