#ifndef NEXUS_DEFINITIONSSERVLET_HPP
#define NEXUS_DEFINITIONSSERVLET_HPP
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"
#include "Nexus/DefinitionsService/DefinitionsServices.hpp"
#include "Nexus/DefinitionsService/DefinitionsSession.hpp"

namespace Nexus {
namespace DefinitionsService {

  /*! \class DefinitionsServlet
      \brief Provides system wide definitions.
      \tparam ContainerType The container instantiating this servlet.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient used to
              manage accounts.
   */
  template<typename ContainerType, typename ServiceLocatorClientType>
  class DefinitionsServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of ServiceLocatorClient used to manage accounts.
      using ServiceLocatorClient = Beam::GetTryDereferenceType<
        ServiceLocatorClientType>;

      //! Constructs a DefinitionsServlet.
      /*!
        \param minimumSpireClientVersion The minimum version of the Spire client
               required to login.
        \param timeZoneDatabase The time zone database.
        \param countryDatabase The CountryDatabase to disseminate.
        \param currencyDatabase The CurrencyDatabase to disseminate.
        \param marketDatabase The MarketDatabase to disseminate.
        \param destinationDatabase The DestinationDatabase to disseminate.
        \param exchangeRates The list of ExchangeRates.
        \param complianceRuleSchemas The list of ComplianceRuleSchemas.
        \param serviceLocatorClient Initializes the ServiceLocatorClient.
      */
      template<typename ServiceLocatorClientForward>
      DefinitionsServlet(std::string minimumSpireClientVersion,
        std::string timeZoneDatabase, CountryDatabase countryDatabase,
        CurrencyDatabase currencyDatabase, MarketDatabase marketDatabase,
        DestinationDatabase destinationDatabase,
        std::vector<ExchangeRate> exchangeRates,
        std::vector<Compliance::ComplianceRuleSchema> complianceRuleSchemas,
        ServiceLocatorClientForward&& serviceLocatorClient);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void Open();

      void Close();

    private:
      std::string m_minimumSpireClientVersion;
      std::string m_timeZoneDatabase;
      CountryDatabase m_countryDatabase;
      CurrencyDatabase m_currencyDatabase;
      DestinationDatabase m_destinationDatabase;
      MarketDatabase m_marketDatabase;
      std::vector<ExchangeRate> m_exchangeRates;
      std::vector<Compliance::ComplianceRuleSchema> m_complianceRuleSchemas;
      Beam::GetOptionalLocalPtr<ServiceLocatorClientType>
        m_serviceLocatorClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      std::string OnLoadMinimumSpireClientVersion(ServiceProtocolClient& client,
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

  template<typename ServiceLocatorClientType>
  struct MetaDefinitionsServlet {
    using Session = DefinitionsSession;
    template<typename ContainerType>
    struct apply {
      using type = DefinitionsServlet<ContainerType, ServiceLocatorClientType>;
    };
  };

  template<typename ContainerType, typename ServiceLocatorClientType>
  template<typename ServiceLocatorClientForward>
  DefinitionsServlet<ContainerType, ServiceLocatorClientType>::
      DefinitionsServlet(std::string minimumSpireClientVersion,
      std::string timeZoneDatabase, CountryDatabase countryDatabase,
      CurrencyDatabase currencyDatabase, MarketDatabase marketDatabase,
      DestinationDatabase destinationDatabase,
      std::vector<ExchangeRate> exchangeRates,
      std::vector<Compliance::ComplianceRuleSchema> complianceRuleSchemas,
      ServiceLocatorClientForward&& serviceLocatorClient)
      : m_minimumSpireClientVersion{std::move(minimumSpireClientVersion)},
        m_timeZoneDatabase{std::move(timeZoneDatabase)},
        m_countryDatabase{std::move(countryDatabase)},
        m_currencyDatabase{std::move(currencyDatabase)},
        m_marketDatabase{std::move(marketDatabase)},
        m_destinationDatabase{std::move(destinationDatabase)},
        m_exchangeRates{std::move(exchangeRates)},
        m_complianceRuleSchemas{std::move(complianceRuleSchemas)},
        m_serviceLocatorClient{std::forward<ServiceLocatorClientForward>(
          serviceLocatorClient)} {}

  template<typename ContainerType, typename ServiceLocatorClientType>
  void DefinitionsServlet<ContainerType, ServiceLocatorClientType>::
      RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterDefinitionsServices(Store(slots));
    LoadMinimumSpireClientVersionService::AddSlot(Store(slots), std::bind(
      &DefinitionsServlet::OnLoadMinimumSpireClientVersion, this,
      std::placeholders::_1, std::placeholders::_2));
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

  template<typename ContainerType, typename ServiceLocatorClientType>
  void DefinitionsServlet<ContainerType, ServiceLocatorClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  void DefinitionsServlet<ContainerType, ServiceLocatorClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  void DefinitionsServlet<ContainerType, ServiceLocatorClientType>::Shutdown() {
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  std::string DefinitionsServlet<ContainerType, ServiceLocatorClientType>::
      OnLoadMinimumSpireClientVersion(ServiceProtocolClient& client,
      int dummy) {
    return m_minimumSpireClientVersion;
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  CountryDatabase DefinitionsServlet<ContainerType, ServiceLocatorClientType>::
      OnLoadCountryDatabase(ServiceProtocolClient& client, int dummy) {
    return m_countryDatabase;
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  std::string DefinitionsServlet<ContainerType, ServiceLocatorClientType>::
      OnLoadTimeZoneDatabase(ServiceProtocolClient& client, int dummy) {
    return m_timeZoneDatabase;
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  CurrencyDatabase DefinitionsServlet<ContainerType, ServiceLocatorClientType>::
      OnLoadCurrencyDatabase(ServiceProtocolClient& client, int dummy) {
    return m_currencyDatabase;
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  DestinationDatabase DefinitionsServlet<ContainerType,
      ServiceLocatorClientType>::OnLoadDestinationDatabase(
      ServiceProtocolClient& client, int dummy) {
    return m_destinationDatabase;
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  MarketDatabase DefinitionsServlet<ContainerType, ServiceLocatorClientType>::
      OnLoadMarketDatabase(ServiceProtocolClient& client, int dummy) {
    return m_marketDatabase;
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  std::vector<ExchangeRate> DefinitionsServlet<ContainerType,
      ServiceLocatorClientType>::OnLoadExchangeRates(
      ServiceProtocolClient& client, int dummy) {
    return m_exchangeRates;
  }

  template<typename ContainerType, typename ServiceLocatorClientType>
  std::vector<Compliance::ComplianceRuleSchema> DefinitionsServlet<
      ContainerType, ServiceLocatorClientType>::OnLoadComplianceRuleSchemas(
      ServiceProtocolClient& client, int dummy) {
    return m_complianceRuleSchemas;
  }
}
}

#endif
