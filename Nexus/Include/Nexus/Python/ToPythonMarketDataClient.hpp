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
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BboQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedBookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<BookQuote> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<SequencedTimeAndSale> queue);
      void query(const SecurityMarketDataQuery& query,
        Beam::ScopedQueueWriter<TimeAndSale> queue);
      std::vector<SecurityInfo> query(const SecurityInfoQuery& query);
      SecuritySnapshot load_snapshot(const Security& security);
      SecurityTechnicals load_technicals(const Security& security);
      std::vector<SecurityInfo> load_security_info_from_prefix(
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
  void ToPythonMarketDataClient<C>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBboQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BboQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedBookQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<BookQuote> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<SequencedTimeAndSale> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::query(const SecurityMarketDataQuery& query,
      Beam::ScopedQueueWriter<TimeAndSale> queue) {
    auto release = Beam::Python::GilRelease();
    m_client->query(query, std::move(queue));
  }

  template<IsMarketDataClient C>
  std::vector<SecurityInfo>
      ToPythonMarketDataClient<C>::query(const SecurityInfoQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_client->query(query);
  }

  template<IsMarketDataClient C>
  SecuritySnapshot ToPythonMarketDataClient<C>::load_snapshot(
      const Security& security) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_snapshot(security);
  }

  template<IsMarketDataClient C>
  SecurityTechnicals ToPythonMarketDataClient<C>::load_technicals(
      const Security& security) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_technicals(security);
  }

  template<IsMarketDataClient C>
  std::vector<SecurityInfo>
      ToPythonMarketDataClient<C>::load_security_info_from_prefix(
        const std::string& prefix) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_security_info_from_prefix(prefix);
  }

  template<IsMarketDataClient C>
  void ToPythonMarketDataClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
