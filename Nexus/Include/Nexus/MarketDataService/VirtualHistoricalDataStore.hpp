#ifndef NEXUS_VIRTUAL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_VIRTUAL_HISTORICAL_DATA_STORE_HPP
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /** Provides a virtual interface to a HistoricalDataStore. */
  class VirtualHistoricalDataStore {
    public:
      virtual ~VirtualHistoricalDataStore() = default;

      virtual boost::optional<SecurityInfo> LoadSecurityInfo(
        const Security& security) = 0;

      virtual std::vector<SecurityInfo> LoadAllSecurityInfo() = 0;

      virtual std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query) = 0;

      virtual std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query) = 0;

      virtual std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query) = 0;

      virtual std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query) = 0;

      virtual std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query) = 0;

      virtual void Store(const SecurityInfo& info) = 0;

      virtual void Store(
        const SequencedMarketOrderImbalance& orderImbalance) = 0;

      virtual void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances) = 0;

      virtual void Store(const SequencedSecurityBboQuote& bboQuote) = 0;

      virtual void Store(
        const std::vector<SequencedSecurityBboQuote>& bboQuotes) = 0;

      virtual void Store(const SequencedSecurityMarketQuote& marketQuote) = 0;

      virtual void Store(
        const std::vector<SequencedSecurityMarketQuote>& marketQuotes) = 0;

      virtual void Store(const SequencedSecurityBookQuote& bookQuote) = 0;

      virtual void Store(
        const std::vector<SequencedSecurityBookQuote>& bookQuotes) = 0;

      virtual void Store(const SequencedSecurityTimeAndSale& timeAndSale) = 0;

      virtual void Store(
        const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualHistoricalDataStore. */
      VirtualHistoricalDataStore() = default;
  };

  /**
   * Wraps a HistoricalDataStore providing it with a virtual interface.
   * @param <C> The HistoricalDataStore to wrap.
   */
  template<typename C>
  class WrapperHistoricalDataStore : public VirtualHistoricalDataStore {
    public:

      /** The HistoricalDataStore to wrap. */
      using DataStore = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperHistoricalDataStore.
       * @param dataStore The HistoricalDataStore to wrap.
       */
      template<typename D>
      WrapperHistoricalDataStore(D&& dataStore);

      boost::optional<SecurityInfo> LoadSecurityInfo(
        const Security& security) override;

      std::vector<SecurityInfo> LoadAllSecurityInfo() override;

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

      void Store(const SecurityInfo& info) override;

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

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<C> m_dataStore;
  };

  /**
   * Wraps a HistoricalDataStore into a VirtualHistoricalDataStore.
   * @param dataStore The data store to wrap.
   */
  template<typename DataStore>
  std::unique_ptr<VirtualHistoricalDataStore> MakeVirtualHistoricalDataStore(
      DataStore&& dataStore) {
    return std::make_unique<WrapperHistoricalDataStore<
      std::decay_t<DataStore>>>(std::forward<DataStore>(dataStore));
  }

  template<typename C>
  template<typename D>
  WrapperHistoricalDataStore<C>::WrapperHistoricalDataStore(D&& dataStore)
    : m_dataStore(std::forward<D>(dataStore)) {}

  template<typename C>
  boost::optional<SecurityInfo> WrapperHistoricalDataStore<C>::LoadSecurityInfo(
      const Security& security) {
    return m_dataStore->LoadSecurityInfo(security);
  }

  template<typename C>
  std::vector<SecurityInfo> WrapperHistoricalDataStore<C>::
      LoadAllSecurityInfo() {
    return m_dataStore->LoadAllSecurityInfo();
  }

  template<typename C>
  std::vector<SequencedOrderImbalance>
      WrapperHistoricalDataStore<C>::LoadOrderImbalances(
      const MarketWideDataQuery& query) {
    return m_dataStore->LoadOrderImbalances(query);
  }

  template<typename C>
  std::vector<SequencedBboQuote> WrapperHistoricalDataStore<C>::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadBboQuotes(query);
  }

  template<typename C>
  std::vector<SequencedBookQuote> WrapperHistoricalDataStore<C>::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadBookQuotes(query);
  }

  template<typename C>
  std::vector<SequencedMarketQuote>
      WrapperHistoricalDataStore<C>::LoadMarketQuotes(
      const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadMarketQuotes(query);
  }

  template<typename C>
  std::vector<SequencedTimeAndSale>
      WrapperHistoricalDataStore<C>::LoadTimeAndSales(
      const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadTimeAndSales(query);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_dataStore->Store(orderImbalance);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_dataStore->Store(orderImbalances); 
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_dataStore->Store(bboQuote);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_dataStore->Store(bboQuotes);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_dataStore->Store(marketQuote);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_dataStore->Store(marketQuotes);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_dataStore->Store(bookQuote);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_dataStore->Store(bookQuotes);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_dataStore->Store(timeAndSale);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_dataStore->Store(timeAndSales);
  }

  template<typename C>
  void WrapperHistoricalDataStore<C>::Close() {
    m_dataStore->Close();
  }
}

#endif
