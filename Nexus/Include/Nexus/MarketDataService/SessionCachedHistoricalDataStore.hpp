#ifndef NEXUS_MARKET_DATA_SESSION_CACHED_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_SESSION_CACHED_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/SessionCachedDataStore.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus {

  /**
   * Caches historical market data for a specified session.
   * @param <D> The underlying data store to cache.
   */
  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  class SessionCachedHistoricalDataStore {
    public:

      /** The type of HistoricalDataStore to cache. */
      using HistoricalDataStore = Beam::dereference_t<D>;

      /**
       * Constructs a SessionCachedHistoricalDataStore.
       * @param data_store Initializes the data store to commit data to.
       * @param block_size The size of a single cache block.
       */
      template<Beam::Initializes<D> DF>
      SessionCachedHistoricalDataStore(DF&& data_store, int block_size);

      ~SessionCachedHistoricalDataStore();

      std::vector<TickerInfo> load_ticker_info(
        const TickerInfoQuery& query);
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
      void store(
        const std::vector<SequencedTickerTimeAndSale>& time_and_sales);
      void close();

    private:
      template<typename T>
      using DataStore = Beam::SessionCachedDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        EvaluatorTranslator>;
      Beam::local_ptr_t<D> m_data_store;
      DataStore<BboQuote> m_bbo_quote_data_store;
      DataStore<BookQuote> m_book_quote_data_store;
      DataStore<TimeAndSale> m_time_and_sale_data_store;
      Beam::OpenState m_open_state;

      SessionCachedHistoricalDataStore(
        const SessionCachedHistoricalDataStore&) = delete;
      SessionCachedHistoricalDataStore& operator =(
        const SessionCachedHistoricalDataStore&) = delete;
  };

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  template<Beam::Initializes<D> DF>
  SessionCachedHistoricalDataStore<D>::SessionCachedHistoricalDataStore(
    DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_bbo_quote_data_store(&*m_data_store, block_size / 10),
      m_book_quote_data_store(&*m_data_store, block_size / 10),
      m_time_and_sale_data_store(&*m_data_store, block_size) {}

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  SessionCachedHistoricalDataStore<D>::~SessionCachedHistoricalDataStore() {
    close();
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<TickerInfo> SessionCachedHistoricalDataStore<D>::
      load_ticker_info(const TickerInfoQuery& query) {
    return m_data_store->load_ticker_info(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(const TickerInfo& info) {
    m_data_store->store(info);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedOrderImbalance> SessionCachedHistoricalDataStore<D>::
      load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_data_store->load_order_imbalances(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_data_store->store(imbalance);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_data_store->store(imbalances);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBboQuote> SessionCachedHistoricalDataStore<D>::
      load_bbo_quotes(const TickerMarketDataQuery& query) {
    return m_bbo_quote_data_store.load(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedTickerBboQuote& quote) {
    m_bbo_quote_data_store.store(quote);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBboQuote>& quotes) {
    m_bbo_quote_data_store.store(quotes);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedBookQuote> SessionCachedHistoricalDataStore<D>::
      load_book_quotes(const TickerMarketDataQuery& query) {
    return m_book_quote_data_store.load(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedTickerBookQuote& quote) {
    m_book_quote_data_store.store(quote);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBookQuote>& quotes) {
    m_book_quote_data_store.store(quotes);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  std::vector<SequencedTimeAndSale> SessionCachedHistoricalDataStore<D>::
      load_time_and_sales(const TickerMarketDataQuery& query) {
    return m_time_and_sale_data_store.load(query);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedTickerTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.store(time_and_sale);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.store(time_and_sales);
  }

  template<typename D> requires IsHistoricalDataStore<Beam::dereference_t<D>>
  void SessionCachedHistoricalDataStore<D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_and_sale_data_store.close();
    m_book_quote_data_store.close();
    m_bbo_quote_data_store.close();
    m_data_store->close();
    m_open_state.close();
  }
}

#endif
