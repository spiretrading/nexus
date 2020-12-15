#ifndef NEXUS_PYTHON_HISTORICAL_DATA_STORE_HPP
#define NEXUS_PYTHON_HISTORICAL_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/MarketDataService/HistoricalDataStoreBox.hpp"

namespace Nexus::MarketDataService {

  /**
   * Wraps a HistoricalDataStore for use with Python.
   * @param <D> The type of HistoricalDataStore to wrap.
   */
  template<typename D>
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
      const DataStore& GetDataStore() const;

      /** Returns the wrapped data store. */
      DataStore& GetDataStore();

      std::vector<SecurityInfo> LoadSecurityInfo(
        const SecurityInfoQuery& query);

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query);

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query);

      void Store(const SecurityInfo& info);

      void Store(const SequencedMarketOrderImbalance& orderImbalance);

      void Store(
        const std::vector<SequencedMarketOrderImbalance>& orderImbalances);

      void Store(const SequencedSecurityBboQuote& bboQuote);

      void Store(const std::vector<SequencedSecurityBboQuote>& bboQuotes);

      void Store(const SequencedSecurityMarketQuote& marketQuote);

      void Store(const std::vector<SequencedSecurityMarketQuote>& marketQuotes);

      void Store(const SequencedSecurityBookQuote& bookQuote);

      void Store(const std::vector<SequencedSecurityBookQuote>& bookQuotes);

      void Store(const SequencedSecurityTimeAndSale& timeAndSale);

      void Store(const std::vector<SequencedSecurityTimeAndSale>& timeAndSales);

      void Close();

    private:
      boost::optional<DataStore> m_dataStore;

      ToPythonHistoricalDataStore(const ToPythonHistoricalDataStore&) = delete;
      ToPythonHistoricalDataStore& operator =(
        const ToPythonHistoricalDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonHistoricalDataStore(DataStore&&) ->
    ToPythonHistoricalDataStore<std::decay_t<DataStore>>;

  template<typename D>
  template<typename... Args, typename>
  ToPythonHistoricalDataStore<D>::ToPythonHistoricalDataStore(Args&&... args)
    : m_dataStore((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename D>
  ToPythonHistoricalDataStore<D>::~ToPythonHistoricalDataStore() {
    auto release = Beam::Python::GilRelease();
    m_dataStore.reset();
  }

  template<typename D>
  const typename ToPythonHistoricalDataStore<D>::DataStore&
      ToPythonHistoricalDataStore<D>::GetDataStore() const {
    return *m_dataStore;
  }

  template<typename D>
  typename ToPythonHistoricalDataStore<D>::DataStore&
      ToPythonHistoricalDataStore<D>::GetDataStore() {
    return *m_dataStore;
  }

  template<typename D>
  std::vector<SecurityInfo> ToPythonHistoricalDataStore<D>::LoadSecurityInfo(
      const SecurityInfoQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadSecurityInfo(query);
  }

  template<typename D>
  std::vector<SequencedOrderImbalance>
      ToPythonHistoricalDataStore<D>::LoadOrderImbalances(
        const MarketWideDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadOrderImbalances(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> ToPythonHistoricalDataStore<D>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadBboQuotes(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote>
      ToPythonHistoricalDataStore<D>::LoadBookQuotes(
        const SecurityMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadBookQuotes(query);
  }

  template<typename D>
  std::vector<SequencedMarketQuote>
      ToPythonHistoricalDataStore<D>::LoadMarketQuotes(
        const SecurityMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadMarketQuotes(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale>
      ToPythonHistoricalDataStore<D>::LoadTimeAndSales(
        const SecurityMarketDataQuery& query) {
    auto release = Beam::Python::GilRelease();
    return m_dataStore->LoadTimeAndSales(query);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(const SecurityInfo& info) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(info);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(orderImbalance);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(orderImbalances);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(bboQuote);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(bboQuotes);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(marketQuote);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(marketQuotes);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(bookQuote);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(bookQuotes);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(timeAndSale);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Store(timeAndSales);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Close() {
    auto release = Beam::Python::GilRelease();
    m_dataStore->Close();
  }
}

#endif
