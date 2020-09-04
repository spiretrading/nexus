#ifndef NEXUS_VIRTUAL_DEFINITIONS_CLIENT_HPP
#define NEXUS_VIRTUAL_DEFINITIONS_CLIENT_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
#include <boost/date_time/local_time/tz_database.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus::DefinitionsService {

  /** Provides a pure virtual interface to a DefinitionsClient. */
  class VirtualDefinitionsClient : private boost::noncopyable {
    public:
      virtual ~VirtualDefinitionsClient() = default;

      virtual std::string LoadMinimumSpireClientVersion() = 0;

      virtual std::string LoadOrganizationName() = 0;

      virtual CountryDatabase LoadCountryDatabase() = 0;

      virtual boost::local_time::tz_database LoadTimeZoneDatabase() = 0;

      virtual CurrencyDatabase LoadCurrencyDatabase() = 0;

      virtual DestinationDatabase LoadDestinationDatabase() = 0;

      virtual MarketDatabase LoadMarketDatabase() = 0;

      virtual std::vector<ExchangeRate> LoadExchangeRates() = 0;

      virtual std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas() = 0;

      virtual TradingSchedule LoadTradingSchedule() = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualDefinitionsClient. */
      VirtualDefinitionsClient() = default;
  };

  /**
   * Wraps a DefinitionsClient providing it with a virtual interface.
   * @param <C> The type of DefinitionsClient to wrap.
   */
  template<typename C>
  class WrapperDefinitionsClient : public VirtualDefinitionsClient {
    public:

      /** The DefinitionsClient to wrap. */
      using Client = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperDefinitionsClient.
       * @param client The DefinitionsClient to wrap.
       */
      template<typename DF>
      explicit WrapperDefinitionsClient(DF&& client);

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

      TradingSchedule LoadTradingSchedule() override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<C> m_client;
  };

  /**
   * Wraps a DefinitionsClient into a VirtualDefinitionsClient.
   * @param client The client to wrap.
   */
  template<typename DefinitionsClient>
  std::unique_ptr<VirtualDefinitionsClient> MakeVirtualDefinitionsClient(
      DefinitionsClient&& client) {
    return std::make_unique<WrapperDefinitionsClient<DefinitionsClient>>(
      std::forward<DefinitionsClient>(client));
  }

  template<typename C>
  template<typename DF>
  WrapperDefinitionsClient<C>::WrapperDefinitionsClient(DF&& client)
    : m_client(std::forward<DF>(client)) {}

  template<typename C>
  std::string WrapperDefinitionsClient<C>::LoadMinimumSpireClientVersion() {
    return m_client->LoadMinimumSpireClientVersion();
  }

  template<typename C>
  std::string WrapperDefinitionsClient<C>::LoadOrganizationName() {
    return m_client->LoadOrganizationName();
  }

  template<typename C>
  CountryDatabase WrapperDefinitionsClient<C>::LoadCountryDatabase() {
    return m_client->LoadCountryDatabase();
  }

  template<typename C>
  boost::local_time::tz_database WrapperDefinitionsClient<C>::
      LoadTimeZoneDatabase() {
    return m_client->LoadTimeZoneDatabase();
  }

  template<typename C>
  CurrencyDatabase WrapperDefinitionsClient<C>::LoadCurrencyDatabase() {
    return m_client->LoadCurrencyDatabase();
  }

  template<typename C>
  DestinationDatabase WrapperDefinitionsClient<C>::LoadDestinationDatabase() {
    return m_client->LoadDestinationDatabase();
  }

  template<typename C>
  MarketDatabase WrapperDefinitionsClient<C>::LoadMarketDatabase() {
    return m_client->LoadMarketDatabase();
  }

  template<typename C>
  std::vector<ExchangeRate> WrapperDefinitionsClient<C>::LoadExchangeRates() {
    return m_client->LoadExchangeRates();
  }

  template<typename C>
  std::vector<Compliance::ComplianceRuleSchema>
      WrapperDefinitionsClient<C>::LoadComplianceRuleSchemas() {
    return m_client->LoadComplianceRuleSchemas();
  }

  template<typename C>
  TradingSchedule WrapperDefinitionsClient<C>::LoadTradingSchedule() {
    return m_client->LoadTradingSchedule();
  }

  template<typename C>
  void WrapperDefinitionsClient<C>::Close() {
    m_client->Close();
  }
}

#endif
