#ifndef NEXUS_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#define NEXUS_PYTHON_MARKET_DATA_FEED_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {

  /**
   * Wraps a MarketDataFeedClient for use with Python.
   * param <C> The type of MarketDataFeedClient to wrap.
   */
  template<IsMarketDataFeedClient C>
  class ToPythonMarketDataFeedClient {
    public:

      /** The type of MarketDataClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonMarketDataFeedClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonMarketDataFeedClient(Args&&... args);

      ~ToPythonMarketDataFeedClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      void add(const SecurityInfo& info);
      void publish(const VenueOrderImbalance& imbalance);
      void publish(const SecurityBboQuote& quote);
      void publish(const SecurityBookQuote& quote);
      void publish(const SecurityTimeAndSale& time_and_sale);
      void add_order(const Security& security, Venue venue,
        const std::string& mpid, bool is_primary_mpid, const std::string& id,
        Side side, Money price, Quantity size,
        boost::posix_time::ptime timestamp);
      void modify_order_size(const std::string& id, Quantity size,
        boost::posix_time::ptime timestamp);
      void offset_order_size(const std::string& id, Quantity delta,
        boost::posix_time::ptime timestamp);
      void modify_order_price(const std::string& id, Money price,
        boost::posix_time::ptime timestamp);
      void remove_order(
        const std::string& id, boost::posix_time::ptime timestamp);
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonMarketDataFeedClient(
        const ToPythonMarketDataFeedClient&) = delete;
      ToPythonMarketDataFeedClient& operator =(
        const ToPythonMarketDataFeedClient&) = delete;
  };

  template<typename Client>
  ToPythonMarketDataFeedClient(Client&&) ->
    ToPythonMarketDataFeedClient<std::remove_cvref_t<Client>>;

  template<IsMarketDataFeedClient C>
  template<typename... Args>
  ToPythonMarketDataFeedClient<C>::ToPythonMarketDataFeedClient(Args&&... args)
    : m_client((pybind11::gil_scoped_release(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsMarketDataFeedClient C>
  ToPythonMarketDataFeedClient<C>::~ToPythonMarketDataFeedClient() {
    auto release = pybind11::gil_scoped_release();
    m_client.reset();
  }

  template<IsMarketDataFeedClient C>
  typename ToPythonMarketDataFeedClient<C>::Client&
      ToPythonMarketDataFeedClient<C>::get() {
    return *m_client;
  }

  template<IsMarketDataFeedClient C>
  const typename ToPythonMarketDataFeedClient<C>::Client&
      ToPythonMarketDataFeedClient<C>::get() const {
    return *m_client;
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::add(const SecurityInfo& info) {
    auto release = pybind11::gil_scoped_release();
    m_client->add(info);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::publish(
      const VenueOrderImbalance& imbalance) {
    auto release = pybind11::gil_scoped_release();
    m_client->publish(imbalance);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::publish(const SecurityBboQuote& quote) {
    auto release = pybind11::gil_scoped_release();
    m_client->publish(quote);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::publish(
      const SecurityBookQuote& quote) {
    auto release = pybind11::gil_scoped_release();
    m_client->publish(quote);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::publish(
      const SecurityTimeAndSale& time_and_sale) {
    auto release = pybind11::gil_scoped_release();
    m_client->publish(time_and_sale);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::add_order(
      const Security& security, Venue venue, const std::string& mpid,
      bool is_primary_mpid, const std::string& id, Side side, Money price,
      Quantity size, boost::posix_time::ptime timestamp) {
    auto release = pybind11::gil_scoped_release();
    m_client->add_order(security, venue, mpid, is_primary_mpid, id, side,
      price, size, timestamp);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::modify_order_size(const std::string& id,
      Quantity size, boost::posix_time::ptime timestamp) {
    auto release = pybind11::gil_scoped_release();
    m_client->modify_order_size(id, size, timestamp);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::offset_order_size(const std::string& id,
      Quantity delta, boost::posix_time::ptime timestamp) {
    auto release = pybind11::gil_scoped_release();
    m_client->offset_order_size(id, delta, timestamp);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::modify_order_price(
      const std::string& id, Money price, boost::posix_time::ptime timestamp) {
    auto release = pybind11::gil_scoped_release();
    m_client->modify_order_price(id, price, timestamp);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::remove_order(
      const std::string& id, boost::posix_time::ptime timestamp) {
    auto release = pybind11::gil_scoped_release();
    m_client->remove_order(id, timestamp);
  }

  template<IsMarketDataFeedClient C>
  void ToPythonMarketDataFeedClient<C>::close() {
    auto release = pybind11::gil_scoped_release();
    m_client->close();
  }
}

#endif
