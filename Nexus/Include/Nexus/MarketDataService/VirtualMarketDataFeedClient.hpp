#ifndef NEXUS_MARKETDATAVIRTUALMARKETFEEDDATACLIENT_HPP
#define NEXUS_MARKETDATAVIRTUALMARKETFEEDDATACLIENT_HPP
#include <string>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class VirtualMarketDataFeedClient
      \brief Provides a pure virtual interface to a MarketDataFeedClient.
   */
  class VirtualMarketDataFeedClient : private boost::noncopyable {
    public:
      virtual ~VirtualMarketDataFeedClient() = default;

      virtual void Add(const SecurityInfo& securityInfo) = 0;

      virtual void PublishOrderImbalance(
        const MarketOrderImbalance& orderImbalance) = 0;

      virtual void PublishBboQuote(const SecurityBboQuote& bboQuote) = 0;

      virtual void PublishMarketQuote(
        const SecurityMarketQuote& marketQuote) = 0;

      virtual void SetBookQuote(const SecurityBookQuote& bookQuote) = 0;

      virtual void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const std::string& id,
        Side side, Money price, Quantity size,
        const boost::posix_time::ptime& timestamp) = 0;

      virtual void ModifyOrderSize(const std::string& id, Quantity size,
        const boost::posix_time::ptime& timestamp) = 0;

      virtual void OffsetOrderSize(const std::string& id, Quantity delta,
        const boost::posix_time::ptime& timestamp) = 0;

      virtual void ModifyOrderPrice(const std::string& id, Money price,
        const boost::posix_time::ptime& timestamp) = 0;

      virtual void DeleteOrder(const std::string& id,
        const boost::posix_time::ptime& timestamp) = 0;

      virtual void PublishTimeAndSale(
        const SecurityTimeAndSale& timeAndSale) = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualMarketDataFeedClient.
      VirtualMarketDataFeedClient() = default;
  };

  /*! \class WrapperMarketDataFeedClient
      \brief Wraps a MarketDataFeedClient providing it with a virtual interface.
      \tparam ClientType The MarketDataFeedClient to wrap.
   */
  template<typename ClientType>
  class WrapperMarketDataFeedClient : public VirtualMarketDataFeedClient {
    public:

      //! The MarketDataFeedClient to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperMarketDataFeedClient.
      /*!
        \param client The MarketDataFeedClient to wrap.
      */
      template<typename MarketDataFeedClientForward>
      WrapperMarketDataFeedClient(MarketDataFeedClientForward&& client);

      virtual ~WrapperMarketDataFeedClient() = default;

      virtual void Add(const SecurityInfo& securityInfo);

      virtual void PublishOrderImbalance(
        const MarketOrderImbalance& orderImbalance);

      virtual void PublishBboQuote(const SecurityBboQuote& bboQuote);

      virtual void PublishMarketQuote(const SecurityMarketQuote& marketQuote);

      virtual void SetBookQuote(const SecurityBookQuote& bookQuote);

      virtual void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const std::string& id,
        Side side, Money price, Quantity size,
        const boost::posix_time::ptime& timestamp);

      virtual void ModifyOrderSize(const std::string& id, Quantity size,
        const boost::posix_time::ptime& timestamp);

      virtual void OffsetOrderSize(const std::string& id, Quantity delta,
        const boost::posix_time::ptime& timestamp);

      virtual void ModifyOrderPrice(const std::string& id, Money price,
        const boost::posix_time::ptime& timestamp);

      virtual void DeleteOrder(const std::string& id,
        const boost::posix_time::ptime& timestamp);

      virtual void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale);

      virtual void Close();

    private:
      Beam::GetOptionalLocalPtr<ClientType> m_client;
  };

  //! Wraps a MarketDataFeedClient into a VirtualMarketDataFeedClient.
  /*!
    \param client The client to wrap.
  */
  template<typename MarketDataFeedClient>
  std::unique_ptr<VirtualMarketDataFeedClient> MakeVirtualMarketDataFeedClient(
      MarketDataFeedClient&& client) {
    return std::make_unique<WrapperMarketDataFeedClient<MarketDataFeedClient>>(
      std::forward<MarketDataFeedClient>(client));
  }

  template<typename MarketDataFeedClientType>
  template<typename MarketDataFeedClientForward>
  WrapperMarketDataFeedClient<MarketDataFeedClientType>::
      WrapperMarketDataFeedClient(MarketDataFeedClientForward&& client)
      : m_client{std::forward<MarketDataFeedClientForward>(client)} {}

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::Add(
      const SecurityInfo& securityInfo) {
    return m_client->Add(securityInfo);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::
      PublishOrderImbalance(const MarketOrderImbalance& orderImbalance) {
    return m_client->PublishOrderImbalance(orderImbalance);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::PublishBboQuote(
      const SecurityBboQuote& bboQuote) {
    return m_client->PublishBboQuote(bboQuote);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::
      PublishMarketQuote(const SecurityMarketQuote& marketQuote) {
    return m_client->PublishMarketQuote(marketQuote);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::SetBookQuote(
      const SecurityBookQuote& bookQuote) {
    return m_client->SetBookQuote(bookQuote);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::AddOrder(
      const Security& security, MarketCode market,
      const std::string& mpid, bool isPrimaryMpid, const std::string& id,
      Side side, Money price, Quantity size,
      const boost::posix_time::ptime& timestamp) {
    return m_client->AddOrder(security, market, mpid, isPrimaryMpid, id, side,
      price, size, timestamp);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::ModifyOrderSize(
      const std::string& id, Quantity size,
      const boost::posix_time::ptime& timestamp) {
    return m_client->ModifyOrderSize(id, size, timestamp);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::OffsetOrderSize(
      const std::string& id, Quantity delta,
      const boost::posix_time::ptime& timestamp) {
    return m_client->OffsetOrderSize(id, delta, timestamp);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::ModifyOrderPrice(
      const std::string& id, Money price,
      const boost::posix_time::ptime& timestamp) {
    return m_client->ModifyOrderPrice(id, price, timestamp);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::DeleteOrder(
      const std::string& id, const boost::posix_time::ptime& timestamp) {
    return m_client->DeleteOrder(id, timestamp);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::
      PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale) {
    return m_client->PublishTimeAndSale(timeAndSale);
  }

  template<typename MarketDataFeedClientType>
  void WrapperMarketDataFeedClient<MarketDataFeedClientType>::Close() {
    return m_client->Close();
  }
}
}

#endif
