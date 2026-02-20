#ifndef NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#define NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /**
   * Wraps a MarketDataClient for use with Python.
   * param <C> The type of MarketDataClient to wrap.
   */
  template<IsMarketDataClient C>
  class ToPythonMarketDataClient {
    public:

      /** The type of MarketDataClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonMarketDataClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonMarketDataClient(Args&&... args);

      ~ToPythonMarketDataClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderImbalance> queue);
      void query(const VenueMarketDataQuery& query,
        Beam::ScopedQueueWriter<OrderImbalance> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const TickerQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(
        const TickerQuery& query, Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<TickerInfo> query(const TickerInfoQuery& query);
      TickerSnapshot load_snapshot(const Ticker& ticker);
      PriceCandlestick load_session_candlestick(const Ticker& ticker);
      std::vector<TickerInfo> load_ticker_info_from_prefix(
        const std::string& prefix);
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonMarketDataClient(const ToPythonMarketDataClient&) = delete;
      ToPythonMarketDataClient& operator =(
        const ToPythonMarketDataClient&) = delete;
  };

  template<typename Client>
  ToPythonMarketDataClient(Client&&) ->
    ToPythonMarketDataClient<std::remove_cvref_t<Client>>;

  template<IsMarketDataClient C>
  template<typename... Args>
  ToPythonMarketDataClient<C>::ToPythonMarketDataClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsMarketDataClient C>
  ToPythonMarketDataClient<C>::~ToPythonMarketDataClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsMarketDataClient C>
  typename ToPythonMarketDataClient<C>::Client&
      ToPythonMarketDataClient<C>::get() {
    return *m_client;
  }

  template<IsMarketDataClient C>
  const typename ToPythonMarketDataClient<C>::Client&
      ToPythonMarketDataClient<C>::get() const {
    return *m_client;
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderImbalance> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const VenueMarketDataQuery& query,
      Beam::ScopedQueueWriter<OrderImbalance> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<BboQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<BookQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const TickerQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(
      const TickerQuery& query, Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  std::vector<TickerInfo> ToPythonMarketDataClient<C>::query(
      const TickerInfoQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_client->query(query);
  }

  template<IsMarketDataClient C>
  TickerSnapshot ToPythonMarketDataClient<C>::load_snapshot(
      const Ticker& ticker) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_snapshot(ticker);
  }

  template<IsMarketDataClient C>
  PriceCandlestick ToPythonMarketDataClient<C>::load_session_candlestick(
      const Ticker& ticker) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_session_candlestick(ticker);
  }

  template<IsMarketDataClient C>
  std::vector<TickerInfo>
      ToPythonMarketDataClient<C>::load_ticker_info_from_prefix(
        const std::string& prefix) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_ticker_info_from_prefix(prefix);
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
