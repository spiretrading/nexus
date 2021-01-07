#ifndef NEXUS_TEST_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_TEST_MARKET_DATA_FEED_CLIENT_HPP
#include <string>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTests.hpp"

namespace Nexus::MarketDataService::Tests {

  /**
   * Implements a MarketDataFeedClient that directly modifies a
   * MarketDataRegistry.
   * @param <R> The type of MarketDataRegistry to update.
   */
  template<typename R>
  class TestMarketDataFeedClient {
    public:

      /** The type of MarketDataRegistry to update. */
      using Registry = R;

      /**
       * Constructs a TestMarketDataFeedClient.
       * @param registry The MarketDataRegistry to update.
       */
      template<typename RF>
      explicit TestMarketDataFeedClient(RF&& registry);

      void Add(const SecurityInfo& securityInfo);

      void Publish(const MarketOrderImbalance& orderImbalance);

      void Publish(const SecurityBboQuote& bboQuote);

      void Publish(const SecurityMarketQuote& marketQuote);

      void Publish(const SecurityBookQuote& bookQuote);

      void Publish(const SecurityTimeAndSale& timeAndSale);

      void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const std::string& id,
        Side side, Money price, Quantity size,
        boost::posix_time::ptime timestamp);

      void ModifyOrderSize(const std::string& id, Quantity size,
        boost::posix_time::ptime timestamp);

      void OffsetOrderSize(const std::string& id, Quantity delta,
        boost::posix_time::ptime timestamp);

      void ModifyOrderPrice(const std::string& id, Money price,
        boost::posix_time::ptime timestamp);

      void DeleteOrder(const std::string& id,
        boost::posix_time::ptime timestamp);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<R> m_registry;

      TestMarketDataFeedClient(const TestMarketDataFeedClient&) = delete;
      TestMarketDataFeedClient& operator =(
        const TestMarketDataFeedClient&) = delete;
  };

  template<typename R>
  template<typename RF>
  TestMarketDataFeedClient<R>::TestMarketDataFeedClient(RF&& registry)
    : m_registry(std::forward<RF>(registry)) {}

  template<typename R>
  void TestMarketDataFeedClient<R>::Add(const SecurityInfo& securityInfo) {
    m_registry->Add(securityInfo);
  }

  template<typename R>
  void TestMarketDataFeedClient<R>::Publish(
      const MarketOrderImbalance& orderImbalance) {
    m_registry->PublishOrderImbalance(orderImbalance, 0);
  }

  template<typename R>
  void TestMarketDataFeedClient<R>::Publish(const SecurityBboQuote& bboQuote) {
    m_registry->PublishBboQuote(bboQuote, 0);
  }

  template<typename R>
  void TestMarketDataFeedClient<R>::Publish(
      const SecurityMarketQuote& marketQuote) {
    m_registry->PublishMarketQuote(marketQuote, 0);
  }

  template<typename R>
  void TestMarketDataFeedClient<R>::Publish(
      const SecurityBookQuote& bookQuote) {
    m_registry->UpdateBookQuote(bookQuote, 0);
  }

  template<typename R>
  void TestMarketDataFeedClient<R>::Publish(
      const SecurityTimeAndSale& timeAndSale) {
    m_registry->PublishTimeAndSale(timeAndSale, 0);
  }

  template<typename R>
  void TestMarketDataFeedClient<R>::AddOrder(const Security& security,
    MarketCode market, const std::string& mpid, bool isPrimaryMpid,
    const std::string& id, Side side, Money price, Quantity size,
    boost::posix_time::ptime timestamp) {}

  template<typename R>
  void TestMarketDataFeedClient<R>::ModifyOrderSize(const std::string& id,
    Quantity size, boost::posix_time::ptime timestamp) {}

  template<typename R>
  void TestMarketDataFeedClient<R>::OffsetOrderSize(const std::string& id,
    Quantity delta, boost::posix_time::ptime timestamp) {}

  template<typename R>
  void TestMarketDataFeedClient<R>::ModifyOrderPrice(const std::string& id,
    Money price, boost::posix_time::ptime timestamp) {}

  template<typename R>
  void TestMarketDataFeedClient<R>::DeleteOrder(const std::string& id,
    boost::posix_time::ptime timestamp) {}

  template<typename R>
  void TestMarketDataFeedClient<R>::Close() {}
}

#endif
