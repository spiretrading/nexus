#ifndef NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_CLIENT_HISTORICAL_DATA_STORE_HPP
#include <memory>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /**
   * Wraps a MarketDataClient for use as a HistoricalDataStore.
   * @param <C> The type of MarketDataClient to wrap.
   */
  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  class ClientHistoricalDataStore {
    public:

      /** The type of MarketDataClient to wrap. */
      using MarketDataClient = Beam::dereference_t<C>;

      /**
       * Constructs a ClientHistoricalDataStore.
       * @param client Initializes the client to wrap.
       */
      template<Beam::Initializes<C> CF>
      explicit ClientHistoricalDataStore(CF&& client);

      ~ClientHistoricalDataStore();

      std::vector<TickerInfo> load_ticker_info(const TickerInfoQuery& query);
      void store(const TickerInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerBboQuote& quote);
      void store(const std::vector<SequencedTickerBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerBookQuote& quote);
      void store(const std::vector<SequencedTickerBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerTimeAndSale& time_and_sale);
      void store(const std::vector<SequencedTickerTimeAndSale>& time_and_sales);
      void close();

    private:
      Beam::local_ptr_t<C> m_client;
      Beam::OpenState m_open_state;

      ClientHistoricalDataStore(const ClientHistoricalDataStore&) = delete;
      ClientHistoricalDataStore& operator =(
        const ClientHistoricalDataStore&) = delete;
      template<typename T, typename Query>
      std::vector<T> submit(const Query& query);
  };

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  ClientHistoricalDataStore<C>::ClientHistoricalDataStore(CF&& client)
    : m_client(std::forward<CF>(client)) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  ClientHistoricalDataStore<C>::~ClientHistoricalDataStore() {
    close();
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  std::vector<TickerInfo> ClientHistoricalDataStore<C>::load_ticker_info(
      const TickerInfoQuery& query) {
    return m_client->query(query);
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(const TickerInfo& info) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  std::vector<SequencedOrderImbalance>
      ClientHistoricalDataStore<C>::load_order_imbalances(
        const VenueMarketDataQuery& query) {
    return submit<SequencedOrderImbalance>(query);
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const SequencedVenueOrderImbalance& imbalance) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedVenueOrderImbalance>& imbalances) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  std::vector<SequencedBboQuote> ClientHistoricalDataStore<C>::load_bbo_quotes(
      const TickerMarketDataQuery& query) {
    return submit<SequencedBboQuote>(query);
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const SequencedTickerBboQuote& quote) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedTickerBboQuote>& quotes) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  std::vector<SequencedBookQuote>
      ClientHistoricalDataStore<C>::load_book_quotes(
        const TickerMarketDataQuery& query) {
    return submit<SequencedBookQuote>(query);
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const SequencedTickerBookQuote& quote) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedTickerBookQuote>& quotes) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  std::vector<SequencedTimeAndSale>
      ClientHistoricalDataStore<C>::load_time_and_sales(
        const TickerMarketDataQuery& query) {
    return submit<SequencedTimeAndSale>(query);
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const SequencedTickerTimeAndSale& time_and_sale) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::store(
    const std::vector<SequencedTickerTimeAndSale>& time_and_sales) {}

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  void ClientHistoricalDataStore<C>::close() {
    m_open_state.close();
  }

  template<typename C> requires IsMarketDataClient<Beam::dereference_t<C>>
  template<typename T, typename Query>
  std::vector<T> ClientHistoricalDataStore<C>::submit(const Query& query) {
    auto queue = std::make_shared<Beam::Queue<T>>();
    if(query.get_range().get_end() == Beam::Sequence::LAST) {
      auto revised_query = query;
      revised_query.set_range(
        query.get_range().get_start(), Beam::Sequence::PRESENT);
      m_client->query(revised_query, queue);
    } else {
      m_client->query(query, queue);
    }
    auto matches = std::vector<T>();
    Beam::flush(queue, std::back_inserter(matches));
    return matches;
  }
}

#endif
