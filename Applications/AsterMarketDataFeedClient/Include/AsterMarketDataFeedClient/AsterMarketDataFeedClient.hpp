#ifndef ASTER_MARKET_DATA_FEED_CLIENT_HPP
#define ASTER_MARKET_DATA_FEED_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include "AsterMarketDataFeedClient/AsterWebClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"

namespace Nexus {

  /**
   * Provides market data updates from Aster to a MarketDataFeedClient.
   * @tparam A The AsterWebClient used to receive market data.
   * @tparam M The MarketDataFeedClient used to publish market data.
   */
  template<typename A, typename M>
  class AsterMarketDataFeedClient {
    public:

      /**
       * The type of MarketDataFeedClient used to update the MarketDataServer.
       */
      using MarketDataFeedClient = Beam::dereference_t<M>;

      /**
       * Constructs an AsterMarketDataFeedClient.
       * @param aster_client The client used to receive Aster market data.
       * @param market_data_feed_client The client used to publish market data.
       */
      template<typename AF, typename MF>
      AsterMarketDataFeedClient(
        AF&& aster_client, MF&& market_data_feed_client);

      ~AsterMarketDataFeedClient();

      void close();

    private:
      Beam::local_ptr_t<A> m_aster_client;
      Beam::local_ptr_t<M> m_market_data_feed_client;
      Beam::RoutineTaskQueue m_tasks;
      Beam::OpenState m_open_state;

      AsterMarketDataFeedClient(const AsterMarketDataFeedClient&) = delete;
      AsterMarketDataFeedClient& operator =(
        const AsterMarketDataFeedClient&) = delete;
      void on_bbo_quote(const Ticker& ticker, const BboQuote& bbo_quote);
      void on_book_quote(const Ticker& ticker, const BookQuote& book_quote);
      void on_time_and_sale(
        const Ticker& ticker, const TimeAndSale& time_and_sale);
  };

  template<typename A, typename M>
  template<typename AF, typename MF>
  AsterMarketDataFeedClient<A, M>::AsterMarketDataFeedClient(
    AF&& aster_client, MF&& market_data_feed_client)
    try : m_aster_client(std::forward<AF>(aster_client)),
          m_market_data_feed_client(std::forward<MF>(market_data_feed_client)) {
      auto ticker = Ticker("QQQUSDT", Venue("ASTR"));
      m_market_data_feed_client->add(TickerInfo(
        ticker, boost::lexical_cast<std::string>(ticker), Instrument(),
        Money::CENT, 1, 1, Money::CENT, 1,  1));
      m_aster_client->subscribe(
        ticker, m_tasks.get_slot<BboQuote>(std::bind_front(
          &AsterMarketDataFeedClient::on_bbo_quote, this, ticker)));
      m_aster_client->subscribe(
        ticker, m_tasks.get_slot<BookQuote>(std::bind_front(
          &AsterMarketDataFeedClient::on_book_quote, this, ticker)));
      m_aster_client->subscribe(
        ticker, m_tasks.get_slot<TimeAndSale>(std::bind_front(
          &AsterMarketDataFeedClient::on_time_and_sale, this, ticker)));
    } catch(const std::exception&) {
      std::throw_with_nested(Beam::ConnectException(
        "Failed to initialize the Aster market data feed client."));
    }

  template<typename A, typename M>
  AsterMarketDataFeedClient<A, M>::~AsterMarketDataFeedClient() {
    close();
  }

  template<typename A, typename M>
  void AsterMarketDataFeedClient<A, M>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_aster_client->close();
    m_market_data_feed_client->close();
    m_open_state.close();
  }

  template<typename A, typename M>
  void AsterMarketDataFeedClient<A, M>::on_bbo_quote(
      const Ticker& ticker, const BboQuote& bbo_quote) {
    m_market_data_feed_client->publish(TickerBboQuote(bbo_quote, ticker));
  }

  template<typename A, typename M>
  void AsterMarketDataFeedClient<A, M>::on_book_quote(
      const Ticker& ticker, const BookQuote& book_quote) {
    m_market_data_feed_client->publish(TickerBookQuote(book_quote, ticker));
  }

  template<typename A, typename M>
  void AsterMarketDataFeedClient<A, M>::on_time_and_sale(
      const Ticker& ticker, const TimeAndSale& time_and_sale) {
    m_market_data_feed_client->publish(
      TickerTimeAndSale(time_and_sale, ticker));
  }
}

#endif
