#ifndef NEXUS_VIRTUALDEFINITIONSCLIENT_HPP
#define NEXUS_VIRTUALDEFINITIONSCLIENT_HPP
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
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus {
namespace DefinitionsService {

  /*! \class VirtualDefinitionsClient
      \brief Provides a pure virtual interface to a DefinitionsClient.
   */
  class VirtualDefinitionsClient : private boost::noncopyable {
    public:
      virtual ~VirtualDefinitionsClient() = default;

      virtual std::string LoadMinimumSpireClientVersion() = 0;

      virtual CountryDatabase LoadCountryDatabase() = 0;

      virtual boost::local_time::tz_database LoadTimeZoneDatabase() = 0;

      virtual CurrencyDatabase LoadCurrencyDatabase() = 0;

      virtual DestinationDatabase LoadDestinationDatabase() = 0;

      virtual MarketDatabase LoadMarketDatabase() = 0;

      virtual std::vector<ExchangeRate> LoadExchangeRates() = 0;

      virtual std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas() = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualDefinitionsClient.
      VirtualDefinitionsClient() = default;
  };

  /*! \class WrapperDefinitionsClient
      \brief Wraps a DefinitionsClient providing it with a virtual interface.
      \tparam ClientType The type of DefinitionsClient to wrap.
   */
  template<typename ClientType>
  class WrapperDefinitionsClient : public VirtualDefinitionsClient {
    public:

      //! The DefinitionsClient to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperDefinitionsClient.
      /*!
        \param client The DefinitionsClient to wrap.
      */
      template<typename DefinitionsClientForward>
      WrapperDefinitionsClient(DefinitionsClientForward&& client);

      virtual std::string LoadMinimumSpireClientVersion();

      virtual CountryDatabase LoadCountryDatabase();

      virtual boost::local_time::tz_database LoadTimeZoneDatabase();

      virtual CurrencyDatabase LoadCurrencyDatabase();

      virtual DestinationDatabase LoadDestinationDatabase();

      virtual MarketDatabase LoadMarketDatabase();

      virtual std::vector<ExchangeRate> LoadExchangeRates();

      virtual std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas();

      virtual void Open();

      virtual void Close();

    private:
      Beam::GetOptionalLocalPtr<ClientType> m_client;
  };

  //! Wraps a DefinitionsClient into a VirtualDefinitionsClient.
  /*!
    \param client The client to wrap.
  */
  template<typename DefinitionsClient>
  std::unique_ptr<VirtualDefinitionsClient> MakeVirtualDefinitionsClient(
      DefinitionsClient&& client) {
    return std::make_unique<WrapperDefinitionsClient<DefinitionsClient>>(
      std::forward<DefinitionsClient>(client));
  }

  template<typename ClientType>
  template<typename DefinitionsClientForward>
  WrapperDefinitionsClient<ClientType>::WrapperDefinitionsClient(
      DefinitionsClientForward&& client)
      : m_client{std::forward<DefinitionsClientForward>(client)} {}

  template<typename ClientType>
  std::string WrapperDefinitionsClient<ClientType>::
      LoadMinimumSpireClientVersion() {
    return m_client->LoadMinimumSpireClientVersion();
  }

  template<typename ClientType>
  CountryDatabase WrapperDefinitionsClient<ClientType>::LoadCountryDatabase() {
    return m_client->LoadCountryDatabase();
  }

  template<typename ClientType>
  boost::local_time::tz_database WrapperDefinitionsClient<ClientType>::
      LoadTimeZoneDatabase() {
    return m_client->LoadTimeZoneDatabase();
  }

  template<typename ClientType>
  CurrencyDatabase WrapperDefinitionsClient<ClientType>::
      LoadCurrencyDatabase() {
    return m_client->LoadCurrencyDatabase();
  }

  template<typename ClientType>
  DestinationDatabase WrapperDefinitionsClient<ClientType>::
      LoadDestinationDatabase() {
    return m_client->LoadDestinationDatabase();
  }

  template<typename ClientType>
  MarketDatabase WrapperDefinitionsClient<ClientType>::LoadMarketDatabase() {
    return m_client->LoadMarketDatabase();
  }

  template<typename ClientType>
  std::vector<ExchangeRate> WrapperDefinitionsClient<ClientType>::
      LoadExchangeRates() {
    return m_client->LoadExchangeRates();
  }

  template<typename ClientType>
  std::vector<Compliance::ComplianceRuleSchema>
      WrapperDefinitionsClient<ClientType>::LoadComplianceRuleSchemas() {
    return m_client->LoadComplianceRuleSchemas();
  }

  template<typename ClientType>
  void WrapperDefinitionsClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperDefinitionsClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
