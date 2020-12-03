#ifndef NEXUS_DEFINITIONS_CLIENT_BOX_HPP
#define NEXUS_DEFINITIONS_CLIENT_BOX_HPP
#include <memory>
#include <string>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/ExchangeRate.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/TradingSchedule.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus::DefinitionsService {

  /** Provides a generic interface over an arbitrary DefinitionsClient. */
  class DefinitionsClientBox {
    public:

      /**
       * Constructs a DefinitionsClientBox of a specified type using
       * emplacement.
       * @param <T> The type of definitions client to emplace.
       * @param args The arguments to pass to the emplaced definitions client.
       */
      template<typename T, typename... Args>
      explicit DefinitionsClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a DefinitionsClientBox by copying an existing definitions
       * client.
       * @param client The client to copy.
       */
      template<typename DefinitionsClient>
      explicit DefinitionsClientBox(DefinitionsClient client);

      explicit DefinitionsClientBox(DefinitionsClientBox* client);

      explicit DefinitionsClientBox(
        const std::shared_ptr<DefinitionsClientBox>& client);

      explicit DefinitionsClientBox(
        const std::unique_ptr<DefinitionsClientBox>& client);

      std::string LoadMinimumSpireClientVersion();

      std::string LoadOrganizationName();

      CountryDatabase LoadCountryDatabase();

      boost::local_time::tz_database LoadTimeZoneDatabase();

      CurrencyDatabase LoadCurrencyDatabase();

      DestinationDatabase LoadDestinationDatabase();

      MarketDatabase LoadMarketDatabase();

      std::vector<ExchangeRate> LoadExchangeRates();

      std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas();

      TradingSchedule LoadTradingSchedule();

      void Close();

    private:
      struct VirtualDefinitionsClient {
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
      };
      template<typename C>
      struct WrappedDefinitionsClient final : VirtualDefinitionsClient {
        using DefinitionsClient = C;
        Beam::GetOptionalLocalPtr<DefinitionsClient> m_client;

        template<typename... Args>
        WrappedDefinitionsClient(Args&&... args);
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
      };
      std::shared_ptr<VirtualDefinitionsClient> m_client;
  };

  template<typename T, typename... Args>
  DefinitionsClientBox::DefinitionsClientBox(
    std::in_place_type_t<T>, Args&&... args)
      : m_client(std::make_shared<WrappedDefinitionsClient<T>>(
          std::forward<Args>(args)...)) {}

  template<typename DefinitionsClient>
  DefinitionsClientBox::DefinitionsClientBox(DefinitionsClient client)
    : DefinitionsClientBox(std::in_place_type<DefinitionsClient>,
        std::move(client)) {}

  inline DefinitionsClientBox::DefinitionsClientBox(
    DefinitionsClientBox* client)
      : DefinitionsClientBox(*client) {}

  inline DefinitionsClientBox::DefinitionsClientBox(
    const std::shared_ptr<DefinitionsClientBox>& client)
      : DefinitionsClientBox(*client) {}

  inline DefinitionsClientBox::DefinitionsClientBox(
    const std::unique_ptr<DefinitionsClientBox>& client)
      : DefinitionsClientBox(*client) {}

  inline std::string DefinitionsClientBox::LoadMinimumSpireClientVersion() {
    return m_client->LoadMinimumSpireClientVersion();
  }

  inline std::string DefinitionsClientBox::LoadOrganizationName() {
    return m_client->LoadOrganizationName();
  }

  inline CountryDatabase DefinitionsClientBox::LoadCountryDatabase() {
    return m_client->LoadCountryDatabase();
  }

  inline boost::local_time::tz_database DefinitionsClientBox::
      LoadTimeZoneDatabase() {
    return m_client->LoadTimeZoneDatabase();
  }

  inline CurrencyDatabase DefinitionsClientBox::LoadCurrencyDatabase() {
    return m_client->LoadCurrencyDatabase();
  }

  inline DestinationDatabase DefinitionsClientBox::LoadDestinationDatabase() {
    return m_client->LoadDestinationDatabase();
  }

  inline MarketDatabase DefinitionsClientBox::LoadMarketDatabase() {
    return m_client->LoadMarketDatabase();
  }

  inline std::vector<ExchangeRate> DefinitionsClientBox::LoadExchangeRates() {
    return m_client->LoadExchangeRates();
  }

  inline std::vector<Compliance::ComplianceRuleSchema>
      DefinitionsClientBox::LoadComplianceRuleSchemas() {
    return m_client->LoadComplianceRuleSchemas();
  }

  inline TradingSchedule DefinitionsClientBox::LoadTradingSchedule() {
    return m_client->LoadTradingSchedule();
  }

  inline void DefinitionsClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  DefinitionsClientBox::WrappedDefinitionsClient<C>::WrappedDefinitionsClient(
    Args&&... args)
      : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::string DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadMinimumSpireClientVersion() {
    return m_client->LoadMinimumSpireClientVersion();
  }

  template<typename C>
  std::string DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadOrganizationName() {
    return m_client->LoadOrganizationName();
  }

  template<typename C>
  CountryDatabase DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadCountryDatabase() {
    return m_client->LoadCountryDatabase();
  }

  template<typename C>
  boost::local_time::tz_database DefinitionsClientBox::
      WrappedDefinitionsClient<C>::LoadTimeZoneDatabase() {
    return m_client->LoadTimeZoneDatabase();
  }

  template<typename C>
  CurrencyDatabase DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadCurrencyDatabase() {
    return m_client->LoadCurrencyDatabase();
  }

  template<typename C>
  DestinationDatabase DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadDestinationDatabase() {
    return m_client->LoadDestinationDatabase();
  }

  template<typename C>
  MarketDatabase DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadMarketDatabase() {
    return m_client->LoadMarketDatabase();
  }

  template<typename C>
  std::vector<ExchangeRate> DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadExchangeRates() {
    return m_client->LoadExchangeRates();
  }

  template<typename C>
  std::vector<Compliance::ComplianceRuleSchema>
      DefinitionsClientBox::WrappedDefinitionsClient<C>::
        LoadComplianceRuleSchemas() {
    return m_client->LoadComplianceRuleSchemas();
  }

  template<typename C>
  TradingSchedule DefinitionsClientBox::WrappedDefinitionsClient<C>::
      LoadTradingSchedule() {
    return m_client->LoadTradingSchedule();
  }

  template<typename C>
  void DefinitionsClientBox::WrappedDefinitionsClient<C>::Close() {
    m_client->Close();
  }
}

#endif
