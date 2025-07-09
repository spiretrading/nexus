#ifndef NEXUS_HISTORICAL_DATA_STORE_HPP
#define NEXUS_HISTORICAL_DATA_STORE_HPP
#include <memory>
#include <utility>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

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
       * @param data_store The data store to copy.
       */
      template<typename DataStore>
      explicit HistoricalDataStoreBox(DataStore data_store);

      explicit HistoricalDataStoreBox(HistoricalDataStoreBox* data_store);

      explicit HistoricalDataStoreBox(
        const std::shared_ptr<HistoricalDataStoreBox>& data_store);

      explicit HistoricalDataStoreBox(
        const std::unique_ptr<HistoricalDataStoreBox>& data_store);

      /**
       * Loads SecurityInfo objects that match a query.
       * @param query The query to load.
       * @return The list of all SecurityInfo objects that match the
       *         <i>query</i>.
       */
      std::vector<SecurityInfo> load_security_info(
        const SecurityInfoQuery& query);

      /**
       * Executes a search query over a Market's OrderImbalances.
       * @param query The search query to execute.
       * @return The list of the OrderImbalances that satisfy the search
       *         <i>query</i>.
       */
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);

      /**
       * Executes a search query over a Security's BboQuotes.
       * @param query The search query to execute.
       * @return The list of BboQuotes that satisfy the search <i>query</i>.
       */
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const SecurityMarketDataQuery& query);

      /**
       * Executes a search query over a Security's BookQuotes.
       * @param query The search query to execute.
       * @return The list of BookQuotes that satisfy the search <i>query</i>.
       */
      std::vector<SequencedBookQuote> load_book_quotes(
        const SecurityMarketDataQuery& query);

      /**
       * Executes a search query over a Security's TimeAndSales.
       * @param query The search query to execute.
       * @return The list of TimeAndSales that satisfy the search <i>query</i>.
       */
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const SecurityMarketDataQuery& query);

      /**
       * Stores a SecurityInfo.
       * @param info The SecurityInfo to store.
       */
      void store(const SecurityInfo& info);

      /**
       * Stores a SequencedSecurityBboQuote.
       * @param bbo_quote The SequencedSecurityBboQuote to store.
       */
      void store(const SequencedSecurityBboQuote& bbo_quote);

      /**
       * Stores a list of SequencedSecurityBboQuotes.
       * @param bbo_quotes The list of SequencedSecurityBboQuotes to store.
       */
      void store(const std::vector<SequencedSecurityBboQuote>& bbo_quotes);

      /**
       * Stores a SequencedSecurityBookQuote.
       * @param book_quote The SequencedSecurityBookQuote to store.
       */
      void store(const SequencedSecurityBookQuote& book_quote);

      /**
       * Stores a list of SequencedSecurityBookQuotes.
       * @param book_quotes The list of SequencedSecurityBookQuotes to store.
       */
      void store(const std::vector<SequencedSecurityBookQuote>& book_quotes);

      /**
       * Stores a SequencedSecurityTimeAndSale.
       * @param time_and_sale The SequencedSecurityTimeAndSale to store.
       */
      void store(const SequencedSecurityTimeAndSale& time_and_sale);

      /**
       * Stores a list of SequencedSecurityTimeAndSales.
       * @param time_and_sales The list of SequencedSecurityTimeAndSales to
       *        store.
       */
      void store(
        const std::vector<SequencedSecurityTimeAndSale>& time_and_sales);

      /** Closes the data store. */
      void close();

    private:
      struct VirtualHistoricalDataStore {
        virtual ~VirtualHistoricalDataStore() = default;
        virtual std::vector<SecurityInfo> load_security_info(
          const SecurityInfoQuery& query) = 0;
        virtual std::vector<SequencedOrderImbalance> load_order_imbalances(
          const VenueMarketDataQuery& query) = 0;
        virtual std::vector<SequencedBboQuote> load_bbo_quotes(
          const SecurityMarketDataQuery& query) = 0;
        virtual std::vector<SequencedBookQuote> load_book_quotes(
          const SecurityMarketDataQuery& query) = 0;
        virtual std::vector<SequencedTimeAndSale> load_time_and_sales(
          const SecurityMarketDataQuery& query) = 0;
        virtual void store(const SecurityInfo& info) = 0;
        virtual void store(const SequencedSecurityBboQuote& bbo_quote) = 0;
        virtual void store(
          const std::vector<SequencedSecurityBboQuote>& bbo_quotes) = 0;
        virtual void store(const SequencedSecurityBookQuote& book_quote) = 0;
        virtual void store(
          const std::vector<SequencedSecurityBookQuote>& book_quotes) = 0;
        virtual void store(
          const SequencedSecurityTimeAndSale& time_and_sale) = 0;
        virtual void store(
          const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedHistoricalDataStore final : VirtualHistoricalDataStore {
        using DataStore = D;
        Beam::GetOptionalLocalPtr<DataStore> m_data_store;

        template<typename... Args>
        WrappedHistoricalDataStore(Args&&... args);
        std::vector<SecurityInfo> load_security_info(
          const SecurityInfoQuery& query) override;
        std::vector<SequencedOrderImbalance> load_order_imbalances(
          const VenueMarketDataQuery& query) override;
        std::vector<SequencedBboQuote> load_bbo_quotes(
          const SecurityMarketDataQuery& query) override;
        std::vector<SequencedBookQuote> load_book_quotes(
          const SecurityMarketDataQuery& query) override;
        std::vector<SequencedTimeAndSale> load_time_and_sales(
          const SecurityMarketDataQuery& query) override;
        void store(const SecurityInfo& info) override;
        void store(const SequencedSecurityBboQuote& bbo_quote) override;
        void store(
          const std::vector<SequencedSecurityBboQuote>& bbo_quotes) override;
        void store(const SequencedSecurityBookQuote& book_quote) override;
        void store(
          const std::vector<SequencedSecurityBookQuote>& book_quotes) override;
        void store(const SequencedSecurityTimeAndSale& time_and_sale) override;
        void store(const std::vector<SequencedSecurityTimeAndSale>&
          time_and_sales) override;
        void close() override;
      };
      std::shared_ptr<VirtualHistoricalDataStore> m_data_store;
  };

  template<typename T, typename... Args>
  HistoricalDataStoreBox::HistoricalDataStoreBox(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(std::make_shared<WrappedHistoricalDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<typename DataStore>
  HistoricalDataStoreBox::HistoricalDataStoreBox(DataStore data_store)
    : HistoricalDataStoreBox(
        std::in_place_type<DataStore>, std::move(data_store)) {}

  inline HistoricalDataStoreBox::HistoricalDataStoreBox(
    HistoricalDataStoreBox* data_store)
    : HistoricalDataStoreBox(*data_store) {}

  inline HistoricalDataStoreBox::HistoricalDataStoreBox(
    const std::shared_ptr<HistoricalDataStoreBox>& data_store)
    : HistoricalDataStoreBox(*data_store) {}

  inline HistoricalDataStoreBox::HistoricalDataStoreBox(
    const std::unique_ptr<HistoricalDataStoreBox>& data_store)
    : HistoricalDataStoreBox(*data_store) {}

  inline std::vector<SecurityInfo> HistoricalDataStoreBox::load_security_info(
      const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  inline std::vector<SequencedOrderImbalance>
      HistoricalDataStoreBox::load_order_imbalances(
        const VenueMarketDataQuery& query) {
    return m_data_store->load_order_imbalances(query);
  }

  inline std::vector<SequencedBboQuote> HistoricalDataStoreBox::load_bbo_quotes(
      const SecurityMarketDataQuery& query) {
    return m_data_store->load_bbo_quotes(query);
  }

  inline std::vector<SequencedBookQuote> HistoricalDataStoreBox::
      load_book_quotes(const SecurityMarketDataQuery& query) {
    return m_data_store->load_book_quotes(query);
  }

  inline std::vector<SequencedTimeAndSale>
      HistoricalDataStoreBox::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_time_and_sales(query);
  }

  inline void HistoricalDataStoreBox::store(const SecurityInfo& info) {
    m_data_store->store(info);
  }

  inline void HistoricalDataStoreBox::store(
      const SequencedSecurityBboQuote& bbo_quote) {
    m_data_store->store(bbo_quote);
  }

  inline void HistoricalDataStoreBox::store(
      const std::vector<SequencedSecurityBboQuote>& bbo_quotes) {
    m_data_store->store(bbo_quotes);
  }

  inline void HistoricalDataStoreBox::store(
      const SequencedSecurityBookQuote& book_quote) {
    m_data_store->store(book_quote);
  }

  inline void HistoricalDataStoreBox::store(
      const std::vector<SequencedSecurityBookQuote>& book_quotes) {
    m_data_store->store(book_quotes);
  }

  inline void HistoricalDataStoreBox::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_data_store->store(time_and_sale);
  }

  inline void HistoricalDataStoreBox::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_data_store->store(time_and_sales);
  }

  inline void HistoricalDataStoreBox::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::
    WrappedHistoricalDataStore(Args&&... args)
    : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  std::vector<SecurityInfo>
      HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::load_security_info(
        const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  template<typename D>
  std::vector<SequencedOrderImbalance>
      HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::
        load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_data_store->load_order_imbalances(query);
  }

  template<typename D>
  std::vector<SequencedBboQuote> HistoricalDataStoreBox::
      WrappedHistoricalDataStore<D>::load_bbo_quotes(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_bbo_quotes(query);
  }

  template<typename D>
  std::vector<SequencedBookQuote> HistoricalDataStoreBox::
      WrappedHistoricalDataStore<D>::load_book_quotes(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_book_quotes(query);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale> HistoricalDataStoreBox::
      WrappedHistoricalDataStore<D>::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_time_and_sales(query);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::store(
      const SecurityInfo& info) {
    m_data_store->store(info);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::store(
      const SequencedSecurityBboQuote& bbo_quote) {
    m_data_store->store(bbo_quote);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBboQuote>& bbo_quotes) {
    m_data_store->store(bbo_quotes);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::store(
      const SequencedSecurityBookQuote& book_quote) {
    m_data_store->store(book_quote);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBookQuote>& book_quotes) {
    m_data_store->store(book_quotes);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_data_store->store(time_and_sale);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_data_store->store(time_and_sales);
  }

  template<typename D>
  void HistoricalDataStoreBox::WrappedHistoricalDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
