#ifndef NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#define NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a MarketDataClient for use with Python.
   * param <C> The type of MarketDataClient to wrap.
   */
  template<typename C>
  class ToPythonMarketDataClient {
    public:

      /** The type of MarketDataClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonMarketDataClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonMarketDataClient, Args...>>
      ToPythonMarketDataClient(Args&&... args);

      ~ToPythonMarketDataClient();

      /** Returns the wrapped client. */
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);

      void QueryOrderImbalances(const MarketWideDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);

      void QueryBboQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);

      void QueryBookQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedMarketQuote> queue);

      void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<MarketQuote> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);

      void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);

      SecuritySnapshot LoadSecuritySnapshot(const Security& security);

      SecurityTechnicals LoadSecurityTechnicals(const Security& security);

      std::vector<SecurityInfo> QuerySecurityInfo(
        const SecurityInfoQuery& query);

      std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix);

      void Close();

    private:
      boost::optional<Client> m_client;

      ToPythonMarketDataClient(const ToPythonMarketDataClient&) = delete;
      ToPythonMarketDataClient& operator =(
        const ToPythonMarketDataClient&) = delete;
  };

  template<typename Client>
  ToPythonMarketDataClient(Client&&) ->
    ToPythonMarketDataClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonMarketDataClient<C>::ToPythonMarketDataClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonMarketDataClient<C>::~ToPythonMarketDataClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonMarketDataClient<C>::Client&
      ToPythonMarketDataClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonMarketDataClient<C>::Client&
      ToPythonMarketDataClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryOrderImbalances(
      const MarketWideDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryOrderImbalances(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBboQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBboQuotes(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryBookQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryBookQuotes(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedMarketQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryMarketQuotes(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<MarketQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryMarketQuotes(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::QueryTimeAndSales(
      const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->QueryTimeAndSales(query, std::move(queue));
  }

  template<typename C>
  SecuritySnapshot ToPythonMarketDataClient<C>::LoadSecuritySnapshot(
      const Security& security) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadSecuritySnapshot(security);
  }

  template<typename C>
  SecurityTechnicals ToPythonMarketDataClient<C>::
      LoadSecurityTechnicals(const Security& security) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadSecurityTechnicals(security);
  }

  template<typename C>
  std::vector<SecurityInfo> ToPythonMarketDataClient<C>::QuerySecurityInfo(
      const SecurityInfoQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_client->QuerySecurityInfo(query);
  }

  template<typename C>
  std::vector<SecurityInfo> ToPythonMarketDataClient<C>::
      LoadSecurityInfoFromPrefix(const std::string& prefix) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadSecurityInfoFromPrefix(prefix);
  }

  template<typename C>
  void ToPythonMarketDataClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
