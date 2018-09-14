#ifndef NEXUS_TO_PYTHON_HISTORICAL_DATA_STORE_HPP
#define NEXUS_TO_PYTHON_HISTORICAL_DATA_STORE_HPP
#include <memory>
#include <thread>
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/VirtualHistoricalDataStore.hpp"

namespace Nexus::MarketDataService {

  /** Wraps a HistoricalDataStore for use with Python.
      \tparam D The type of HistoricalDataStore to wrap.
   */
  template<typename D>
  class ToPythonHistoricalDataStore final : public VirtualMarketDataClient {
    public:

      //! The type of DataStore to wrap.
      using DataStore = D;

      //! Constructs a ToPythonHistoricalDataStore.
      /*!
        \param dataStore The data store to wrap.
      */
      ToPythonHistoricalDataStore(std::unique_ptr<DataStore> dataStore);

      virtual ~ToPythonHistoricalDataStore() override;

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query) override;

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query) override;

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query) override;

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query) override;

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query) override;

      void Store(const SequencedMarketOrderImbalance& orderImbalance) override;

      void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances) override;

      void Store(const SequencedSecurityBboQuote& bboQuote) override;

      void Store(
        const std::vector<SequencedSecurityBboQuote>& bboQuotes) override;

      void Store(const SequencedSecurityMarketQuote& marketQuote) override;

      void Store(
        const std::vector<SequencedSecurityMarketQuote>& marketQuotes) override;

      void Store(const SequencedSecurityBookQuote& bookQuote) override;

      void Store(
        const std::vector<SequencedSecurityBookQuote>& bookQuotes) override;

      void Store(const SequencedSecurityTimeAndSale& timeAndSale) override;

      void Store(
        const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) override;

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<DataStore> m_dataStore;
  };

  //! Makes a ToPythonHistoricalDataStore.
  /*!
    \param dataStore The data store to wrap.
  */
  template<typename DataStore>
  auto MakeToPythonHistoricalDataStore(std::unique_ptr<DataStore> dataStore) {
    return std::make_unique<ToPythonHistoricalDataStore<DataStore>>(
      std::move(dataStore));
  }

  template<typename D>
  ToPythonHistoricalDataStore<D>::ToPythonHistoricalDataStore(
      std::unique_ptr<DataStore> dataStore)
      : m_dataStore(std::move(dataStore)) {}

  template<typename D>
  ToPythonHistoricalDataStore<D>::~ToPythonHistoricalDataStore() {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    Close();
    m_client.reset();
  }

  template<typename D>
  std::vector<SequencedOrderImbalance>
      ToPythonHistoricalDataStore<D>::LoadOrderImbalances(
      const MarketWideDataQuery& query) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    return m_client->LoadOrderImbalances(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> ToPythonHistoricalDataStore<D>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    return m_client->LoadBboQuotes(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote>
      ToPythonHistoricalDataStore<D>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    return m_client->LoadBookQuotes(query);
  }

  template<typename D>
  std::vector<SequencedMarketQuote>
      ToPythonHistoricalDataStore<D>::LoadMarketQuotes(
      const SecurityMarketDataQuery& query) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    return m_client->LoadMarketQuotes(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale>
      ToPythonHistoricalDataStore<D>::LoadTimeAndSales(
      const SecurityMarketDataQuery& query) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    return m_client->LoadTimeAndSales(query);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(orderImbalance);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(orderImbalances);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(bboQuote);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(bboQuotes);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(marketQuote);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(marketQuotes);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(bookQuote);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(bookQuotes);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(timeAndSale);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Store(timeAndSales);
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Open() {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Open();
  }

  template<typename D>
  void ToPythonHistoricalDataStore<D>::Close() {
    Beam::Python::GilRelease gil;
    auto release = std::lock_guard(gil);
    m_client->Close();
  }
}

#endif
