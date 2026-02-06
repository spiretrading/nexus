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
      void on_bbo_quote(const Security& security, const BboQuote& bbo_quote);
      void on_book_quote(const Security& security, const BookQuote& book_quote);
      void on_time_and_sale(
        const Security& security, const TimeAndSale& time_and_sale);
  };

  template<typename A, typename M>
  template<typename AF, typename MF>
  AsterMarketDataFeedClient<A, M>::AsterMarketDataFeedClient(
    AF&& aster_client, MF&& market_data_feed_client)
    try : m_aster_client(std::forward<AF>(aster_client)),
          m_market_data_feed_client(std::forward<MF>(market_data_feed_client)) {
      auto security = Security("QQQUSDT", Venue("ASTR"));
      m_aster_client->subscribe(
        security, m_tasks.get_slot<BboQuote>(std::bind_front(
          &AsterMarketDataFeedClient::on_bbo_quote, this, security)));
      m_aster_client->subscribe(
        security, m_tasks.get_slot<BookQuote>(std::bind_front(
          &AsterMarketDataFeedClient::on_book_quote, this, security)));
      m_aster_client->subscribe(
        security, m_tasks.get_slot<TimeAndSale>(std::bind_front(
          &AsterMarketDataFeedClient::on_time_and_sale, this, security)));
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
      const Security& security, const BboQuote& bbo_quote) {
    std::cout << "Bbo Quote: " << security << " " << bbo_quote << std::endl;
    m_market_data_feed_client->publish(SecurityBboQuote(bbo_quote, security));
  }

  template<typename A, typename M>
  void AsterMarketDataFeedClient<A, M>::on_book_quote(
      const Security& security, const BookQuote& book_quote) {
    std::cout << "Book Quote: " << security << " " << book_quote << std::endl;
    m_market_data_feed_client->publish(SecurityBookQuote(book_quote, security));
  }

  template<typename A, typename M>
  void AsterMarketDataFeedClient<A, M>::on_time_and_sale(
      const Security& security, const TimeAndSale& time_and_sale) {
    std::cout << "Time and Sale: " << security << " " << time_and_sale <<
      std::endl;
    m_market_data_feed_client->publish(
      SecurityTimeAndSale(time_and_sale, security));
  }
}

#endif
