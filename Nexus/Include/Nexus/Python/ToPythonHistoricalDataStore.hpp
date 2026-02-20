#ifndef NEXUS_PYTHON_HISTORICAL_DATA_STORE_HPP
#define NEXUS_PYTHON_HISTORICAL_DATA_STORE_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus {

  /**
   * Wraps a HistoricalDataStore for use with Python.
   * @param <D> The type of HistoricalDataStore to wrap.
   */
  template<IsHistoricalDataStore D>
  class ToPythonHistoricalDataStore {
    public:

      /** The type of DataStore to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonHistoricalDataStore in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonHistoricalDataStore(Args&&... args);

      ~ToPythonHistoricalDataStore();

      /** Returns a reference to the underlying data store. */
      DataStore& get();

      /** Returns a reference to the underlying data store. */
      const DataStore& get() const;

      std::vector<TickerInfo> load_ticker_info(const TickerInfoQuery& query);
      void store(const TickerInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(const TickerQuery& query);
      void store(const SequencedTickerBboQuote& quote);
      void store(const std::vector<SequencedTickerBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const TickerQuery& query);
      void store(const SequencedTickerBookQuote& quote);
      void store(const std::vector<SequencedTickerBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const TickerQuery& query);
      void store(const SequencedTickerTimeAndSale& time_and_sale);
      void store(const std::vector<SequencedTickerTimeAndSale>& time_and_sales);
      void close();

    private:
      boost::optional<DataStore> m_data_store;

      ToPythonHistoricalDataStore(const ToPythonHistoricalDataStore&) = delete;
      ToPythonHistoricalDataStore& operator =(
        const ToPythonHistoricalDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonHistoricalDataStore(DataStore&&) ->
    ToPythonHistoricalDataStore<std::remove_cvref_t<DataStore>>;

  template<IsHistoricalDataStore D>
  template<typename... Args>
  ToPythonHistoricalDataStore<D>::ToPythonHistoricalDataStore(Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsHistoricalDataStore D>
  ToPythonHistoricalDataStore<D>::~ToPythonHistoricalDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<IsHistoricalDataStore D>
  typename ToPythonHistoricalDataStore<D>::DataStore&
      ToPythonHistoricalDataStore<D>::get() {
    return *m_data_store;
  }

  template<IsHistoricalDataStore D>
  const typename ToPythonHistoricalDataStore<D>::DataStore&
      ToPythonHistoricalDataStore<D>::get() const {
    return *m_data_store;
  }

  template<IsHistoricalDataStore D>
  std::vector<TickerInfo> ToPythonHistoricalDataStore<D>::load_ticker_info(
      const TickerInfoQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_ticker_info(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(const TickerInfo& info) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(info);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedOrderImbalance>
      ToPythonHistoricalDataStore<D>::load_order_imbalances(
        const VenueMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_order_imbalances(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(imbalance);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(imbalances);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedBboQuote>
      ToPythonHistoricalDataStore<D>::load_bbo_quotes(
        const TickerQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_bbo_quotes(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const SequencedTickerBboQuote& quote) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quote);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBboQuote>& quotes) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedBookQuote>
      ToPythonHistoricalDataStore<D>::load_book_quotes(
        const TickerQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_book_quotes(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const SequencedTickerBookQuote& quote) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quote);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBookQuote>& quotes) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedTimeAndSale>
      ToPythonHistoricalDataStore<D>::load_time_and_sales(
        const TickerQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_time_and_sales(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const SequencedTickerTimeAndSale& time_and_sale) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(time_and_sale);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerTimeAndSale>& time_and_sales) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(time_and_sales);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::close() {
    auto release = Beam::Python::GilRelease();
    m_data_store->close();
  }
}

#endif
