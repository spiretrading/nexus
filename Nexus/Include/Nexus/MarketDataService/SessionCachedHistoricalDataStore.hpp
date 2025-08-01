#ifndef NEXUS_MARKET_DATA_SESSION_CACHED_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_SESSION_CACHED_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/SessionCachedDataStore.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /**
   * Caches historical market data for a specified session.
   * @param <D> The underlying data store to cache.
   */
  template<IsHistoricalDataStore D>
  class SessionCachedHistoricalDataStore {
    public:

      /** The type of HistoricalDataStore to cache. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a SessionCachedHistoricalDataStore.
       * @param data_store Initializes the data store to commit data to.
       * @param block_size The size of a single cache block.
       */
      template<Beam::Initializes<D> DF>
      SessionCachedHistoricalDataStore(DF&& data_store, int block_size);
      ~SessionCachedHistoricalDataStore();
      std::vector<SecurityInfo> load_security_info(
        const SecurityInfoQuery& query);
      void store(const SecurityInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBboQuote& quote);
      void store(const std::vector<SequencedSecurityBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBookQuote& quote);
      void store(const std::vector<SequencedSecurityBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityTimeAndSale& time_and_sale);
      void store(
        const std::vector<SequencedSecurityTimeAndSale>& time_and_sales);
      void close();

    private:
      template<typename T>
      using DataStore = Beam::Queries::SessionCachedDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        Queries::EvaluatorTranslator>;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      DataStore<BboQuote> m_bbo_quote_data_store;
      DataStore<BookQuote> m_book_quote_data_store;
      DataStore<TimeAndSale> m_time_and_sale_data_store;
      Beam::IO::OpenState m_open_state;

      SessionCachedHistoricalDataStore(
        const SessionCachedHistoricalDataStore&) = delete;
      SessionCachedHistoricalDataStore& operator =(
        const SessionCachedHistoricalDataStore&) = delete;
  };

  template<IsHistoricalDataStore D>
  template<Beam::Initializes<D> DF>
  SessionCachedHistoricalDataStore<D>::SessionCachedHistoricalDataStore(
    DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_bbo_quote_data_store(&*m_data_store, block_size / 10),
      m_book_quote_data_store(&*m_data_store, block_size / 10),
      m_time_and_sale_data_store(&*m_data_store, block_size) {}

  template<IsHistoricalDataStore D>
  SessionCachedHistoricalDataStore<D>::~SessionCachedHistoricalDataStore() {
    close();
  }

  template<IsHistoricalDataStore D>
  std::vector<SecurityInfo> SessionCachedHistoricalDataStore<D>::
      load_security_info(const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(const SecurityInfo& info) {
    m_data_store->store(info);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedOrderImbalance> SessionCachedHistoricalDataStore<D>::
      load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_data_store->load_order_imbalances(query);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_data_store->store(imbalance);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_data_store->store(imbalances);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedBboQuote> SessionCachedHistoricalDataStore<D>::
      load_bbo_quotes(const SecurityMarketDataQuery& query) {
    return m_bbo_quote_data_store.Load(query);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedSecurityBboQuote& quote) {
    m_bbo_quote_data_store.Store(quote);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_bbo_quote_data_store.Store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedBookQuote> SessionCachedHistoricalDataStore<D>::
      load_book_quotes(const SecurityMarketDataQuery& query) {
    return m_book_quote_data_store.Load(query);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedSecurityBookQuote& quote) {
    m_book_quote_data_store.Store(quote);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_book_quote_data_store.Store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedTimeAndSale> SessionCachedHistoricalDataStore<D>::
      load_time_and_sales(const SecurityMarketDataQuery& query) {
    return m_time_and_sale_data_store.Load(query);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.Store(time_and_sale);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.Store(time_and_sales);
  }

  template<IsHistoricalDataStore D>
  void SessionCachedHistoricalDataStore<D>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_time_and_sale_data_store.Close();
    m_book_quote_data_store.Close();
    m_bbo_quote_data_store.Close();
    m_data_store->close();
    m_open_state.Close();
  }
}

#endif
