#ifndef NEXUS_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/MarketDataFeedClientBox.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a MarketDataFeedClient for use with Python.
   * param <C> The type of MarketDataFeedClient to wrap.
   */
  template<typename C>
  class ToPythonMarketDataFeedClient {
    public:

      /** The type of MarketDataClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonMarketDataFeedClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonMarketDataFeedClient, Args...>>
      ToPythonMarketDataFeedClient(Args&&... args);

      ~ToPythonMarketDataFeedClient();

      /** Returns the wrapped client. */
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

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
      boost::optional<Client> m_client;

      ToPythonMarketDataFeedClient(
        const ToPythonMarketDataFeedClient&) = delete;
      ToPythonMarketDataFeedClient& operator =(
        const ToPythonMarketDataFeedClient&) = delete;
  };

  template<typename Client>
  ToPythonMarketDataFeedClient(Client&&) ->
    ToPythonMarketDataFeedClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonMarketDataFeedClient<C>::ToPythonMarketDataFeedClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonMarketDataFeedClient<C>::~ToPythonMarketDataFeedClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonMarketDataFeedClient<C>::Client&
      ToPythonMarketDataFeedClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonMarketDataFeedClient<C>::Client&
      ToPythonMarketDataFeedClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Add(const SecurityInfo& securityInfo) {
    auto release = Beam::Python::GilRelease();
    m_client->Add(securityInfo);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Publish(
      const MarketOrderImbalance& orderImbalance) {
    auto release = Beam::Python::GilRelease();
    m_client->Publish(orderImbalance);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Publish(
      const SecurityBboQuote& bboQuote) {
    auto release = Beam::Python::GilRelease();
    m_client->Publish(bboQuote);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Publish(
      const SecurityMarketQuote& marketQuote) {
    auto release = Beam::Python::GilRelease();
    m_client->Publish(marketQuote);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Publish(
      const SecurityBookQuote& bookQuote) {
    auto release = Beam::Python::GilRelease();
    m_client->Publish(bookQuote);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Publish(
      const SecurityTimeAndSale& timeAndSale) {
    auto release = Beam::Python::GilRelease();
    m_client->Publish(timeAndSale);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::AddOrder(const Security& security,
      MarketCode market, const std::string& mpid, bool isPrimaryMpid,
      const std::string& id, Side side, Money price, Quantity size,
      boost::posix_time::ptime timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->AddOrder(security, market, mpid, isPrimaryMpid, id, side, price,
      size, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::ModifyOrderSize(const std::string& id,
      Quantity size, boost::posix_time::ptime timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->ModifyOrderSize(id, size, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::OffsetOrderSize(const std::string& id,
      Quantity delta, boost::posix_time::ptime timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->OffsetOrderSize(id, delta, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::ModifyOrderPrice(const std::string& id,
      Money price, boost::posix_time::ptime timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->ModifyOrderPrice(id, price, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::DeleteOrder(const std::string& id,
      boost::posix_time::ptime timestamp) {
    auto release = Beam::Python::GilRelease();
    m_client->DeleteOrder(id, timestamp);
  }

  template<typename C>
  void ToPythonMarketDataFeedClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
