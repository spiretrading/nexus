#ifndef NEXUS_PYTHON_HISTORICAL_DATA_STORE_HPP
#define NEXUS_PYTHON_HISTORICAL_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus::MarketDataService {

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
       * Constructs a ToPythonHistoricalDataStore.
       * @param args The arguments to forward to the DataStore's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonHistoricalDataStore, Args...>>
      ToPythonHistoricalDataStore(Args&&... args);

      ~ToPythonHistoricalDataStore();

      /** Returns the wrapped data store. */
      const DataStore& get_data_store() const;

      /** Returns the wrapped data store. */
      DataStore& get_data_store();

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
      boost::optional<DataStore> m_data_store;

      ToPythonHistoricalDataStore(const ToPythonHistoricalDataStore&) = delete;
      ToPythonHistoricalDataStore& operator =(
        const ToPythonHistoricalDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonHistoricalDataStore(DataStore&&) ->
    ToPythonHistoricalDataStore<std::remove_reference_t<DataStore>>;

  template<IsHistoricalDataStore D>
  template<typename... Args, typename>
  ToPythonHistoricalDataStore<D>::ToPythonHistoricalDataStore(Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsHistoricalDataStore D>
  ToPythonHistoricalDataStore<D>::~ToPythonHistoricalDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<IsHistoricalDataStore D>
  const typename ToPythonHistoricalDataStore<D>::DataStore&
      ToPythonHistoricalDataStore<D>::get_data_store() const {
    return *m_data_store;
  }

  template<IsHistoricalDataStore D>
  typename ToPythonHistoricalDataStore<D>::DataStore&
      ToPythonHistoricalDataStore<D>::get_data_store() {
    return *m_data_store;
  }

  template<IsHistoricalDataStore D>
  std::vector<SecurityInfo> ToPythonHistoricalDataStore<D>::load_security_info(
      const SecurityInfoQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_security_info(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(const SecurityInfo& info) {
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
        const SecurityMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_bbo_quotes(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const SequencedSecurityBboQuote& quote) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quote);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedBookQuote>
      ToPythonHistoricalDataStore<D>::load_book_quotes(
        const SecurityMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_book_quotes(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const SequencedSecurityBookQuote& quote) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quote);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(quotes);
  }

  template<IsHistoricalDataStore D>
  std::vector<SequencedTimeAndSale>
      ToPythonHistoricalDataStore<D>::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_time_and_sales(query);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(time_and_sale);
  }

  template<IsHistoricalDataStore D>
  void ToPythonHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
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
