#ifndef NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#define NEXUS_PYTHON_MARKET_DATA_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus::MarketDataService {

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
       * Constructs a ToPythonMarketDataClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonMarketDataClient, Args...>>
      ToPythonMarketDataClient(Args&&... args);

      ~ToPythonMarketDataClient();

      /** Returns the wrapped client. */
      const Client& get_client() const;

      /** Returns the wrapped client. */
      Client& get_client();

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
    ToPythonMarketDataClient<std::remove_reference_t<Client>>;

  template<IsMarketDataClient C>
  template<typename... Args, typename>
  ToPythonMarketDataClient<C>::ToPythonMarketDataClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsMarketDataClient C>
  ToPythonMarketDataClient<C>::~ToPythonMarketDataClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsMarketDataClient C>
  const typename ToPythonMarketDataClient<C>::Client&
      ToPythonMarketDataClient<C>::get_client() const {
    return *m_client;
  }

  template<IsMarketDataClient C>
  typename ToPythonMarketDataClient<C>::Client&
      ToPythonMarketDataClient<C>::get_client() {
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
