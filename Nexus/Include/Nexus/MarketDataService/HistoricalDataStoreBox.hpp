#ifndef NEXUS_HISTORICAL_DATA_STORE_BOX_HPP
#define NEXUS_HISTORICAL_DATA_STORE_BOX_HPP
#include <memory>
#include <type_traits>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus::MarketDataService {

  /** Provides a generic interface over an arbitrary HistoricalDataStore. */
  class HistoricalDataStoreBox {
    public:

      /**
       * Constructs an HistoricalDataStoreBox of a specified type using
       * emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit HistoricalDataStoreBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an HistoricalDataStoreBox by copying an existing data store.
       * @param dataStore The data store to copy.
       */
      template<typename DataStore>
      explicit HistoricalDataStoreBox(DataStore dataStore);

      explicit HistoricalDataStoreBox(HistoricalDataStoreBox* dataStore);

      explicit HistoricalDataStoreBox(
        const std::shared_ptr<HistoricalDataStoreBox>& dataStore);

      explicit HistoricalDataStoreBox(
        const std::unique_ptr<HistoricalDataStoreBox>& dataStore);

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
      struct VirtualHistoricalDataStore {
        virtual ~VirtualHistoricalDataStore() = default;
        virtual std::vector<SecurityInfo> LoadSecurityInfo(
          const SecurityInfoQuery& query) = 0;
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
      };
      template<typename D>
      struct WrappedHistoricalDataStore final : VirtualHistoricalDataStore {
        using DataStore = D;
        Beam::GetOptionalLocalPtr<DataStore> m_dataStore;

        template<typename... Args>
        WrappedHistoricalDataStore(Args&&... args);
        std::vector<SecurityInfo> LoadSecurityInfo(
          const SecurityInfoQuery& query) override;
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
        void Store(
          const SequencedMarketOrderImbalance& orderImbalance) override;
        void Store(const std::vector<SequencedMarketOrderImbalance>&
          orderImbalances) override;
        void Store(const SequencedSecurityBboQuote& bboQuote) override;
        void Store(
          const std::vector<SequencedSecurityBboQuote>& bboQuotes) override;
        void Store(const SequencedSecurityMarketQuote& marketQuote) override;
        void Store(const std::vector<SequencedSecurityMarketQuote>&
          marketQuotes) override;
        void Store(const SequencedSecurityBookQuote& bookQuote) override;
        void Store(
          const std::vector<SequencedSecurityBookQuote>& bookQuotes) override;
        void Store(const SequencedSecurityTimeAndSale& timeAndSale) override;
        void Store(const std::vector<SequencedSecurityTimeAndSale>&
          timeAndSales) override;
        void Close() override;
      };
      std::shared_ptr<VirtualHistoricalDataStore> m_dataStore;
  };

  template<typename T, typename... Args>
  HistoricalDataStoreBox::HistoricalDataStoreBox(
    std::in_place_type_t<T>, Args&&... args)
      : m_dataStore(std::make_shared<WrappedHistoricalDataStore<T>>(
          std::forward<Args>(args)...)) {}

  template<typename DataStore>
  HistoricalDataStoreBox::HistoricalDataStoreBox(DataStore dataStore)
    : HistoricalDataStoreBox(std::in_place_type<DataStore>,
        std::move(dataStore)) {}

  inline HistoricalDataStoreBox::HistoricalDataStoreBox(
    HistoricalDataStoreBox* dataStore)
      : HistoricalDataStoreBox(*dataStore) {}

  inline HistoricalDataStoreBox::HistoricalDataStoreBox(
    const std::shared_ptr<HistoricalDataStoreBox>& dataStore)
      : HistoricalDataStoreBox(*dataStore) {}

  inline HistoricalDataStoreBox::HistoricalDataStoreBox(
    const std::unique_ptr<HistoricalDataStoreBox>& dataStore)
      : HistoricalDataStoreBox(*dataStore) {}

  inline std::vector<SecurityInfo> HistoricalDataStoreBox::LoadSecurityInfo(
      const SecurityInfoQuery& query) {
    return m_dataStore->LoadSecurityInfo(query);
  }

  inline std::vector<SequencedOrderImbalance>
      HistoricalDataStoreBox::LoadOrderImbalances(
        const MarketWideDataQuery& query) {
    return m_dataStore->LoadOrderImbalances(query);
  }

  inline std::vector<SequencedBboQuote> HistoricalDataStoreBox::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadBboQuotes(query);
  }

  inline std::vector<SequencedBookQuote> HistoricalDataStoreBox::LoadBookQuotes(
      const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadBookQuotes(query);
  }

  inline std::vector<SequencedMarketQuote>
      HistoricalDataStoreBox::LoadMarketQuotes(
        const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadMarketQuotes(query);
  }

  inline std::vector<SequencedTimeAndSale>
      HistoricalDataStoreBox::LoadTimeAndSales(
        const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadTimeAndSales(query);
  }

  inline void HistoricalDataStoreBox::Store(const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  inline void HistoricalDataStoreBox::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_dataStore->Store(orderImbalance);
  }

  inline void HistoricalDataStoreBox::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_dataStore->Store(orderImbalances); 
  }

  inline void HistoricalDataStoreBox::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_dataStore->Store(bboQuote);
  }

  inline void HistoricalDataStoreBox::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_dataStore->Store(bboQuotes);
  }

  inline void HistoricalDataStoreBox::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_dataStore->Store(marketQuote);
  }

  inline void HistoricalDataStoreBox::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_dataStore->Store(marketQuotes);
  }

  inline void HistoricalDataStoreBox::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_dataStore->Store(bookQuote);
  }

  inline void HistoricalDataStoreBox::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_dataStore->Store(bookQuotes);
  }

  inline void HistoricalDataStoreBox::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_dataStore->Store(timeAndSale);
  }

  inline void HistoricalDataStoreBox::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_dataStore->Store(timeAndSales);
  }

  inline void HistoricalDataStoreBox::Close() {
    m_dataStore->Close();
  }

  template<typename D>
  template<typename... Args>
  HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::
    WrappedHistoricalDataStore(Args&&... args)
      : m_dataStore(std::forward<Args>(args)...) {}

  template<typename D>
  std::vector<SecurityInfo>
      HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::LoadSecurityInfo(
        const SecurityInfoQuery& query) {
    return m_dataStore->LoadSecurityInfo(query);
  }

  template<typename D>
  std::vector<SequencedOrderImbalance>
      HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::
        LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_dataStore->LoadOrderImbalances(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> HistoricalDataStoreBox::
      WrappedHistoricalDataStore<D>::LoadBboQuotes(
        const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadBboQuotes(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote> HistoricalDataStoreBox::
      WrappedHistoricalDataStore<D>::LoadBookQuotes(
        const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadBookQuotes(query);
  }

  template<typename D>
  std::vector<SequencedMarketQuote>
      HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::LoadMarketQuotes(
        const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadMarketQuotes(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale>
      HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::LoadTimeAndSales(
        const SecurityMarketDataQuery& query) {
    return m_dataStore->LoadTimeAndSales(query);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_dataStore->Store(orderImbalance);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_dataStore->Store(orderImbalances); 
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_dataStore->Store(bboQuote);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_dataStore->Store(bboQuotes);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_dataStore->Store(marketQuote);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_dataStore->Store(marketQuotes);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_dataStore->Store(bookQuote);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_dataStore->Store(bookQuotes);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_dataStore->Store(timeAndSale);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_dataStore->Store(timeAndSales);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::Close() {
    m_dataStore->Close();
  }
}

#endif
