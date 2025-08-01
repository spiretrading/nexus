#ifndef NEXUS_HISTORICAL_DATA_STORE_HPP
#define NEXUS_HISTORICAL_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus::MarketDataService {

  /** Provides a generic interface over an arbitrary HistoricalDataStore. */
  class HistoricalDataStore {
    public:

      /**
       * Constructs an HistoricalDataStore of a specified type using
       * emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit HistoricalDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an HistoricalDataStore by copying an existing data store.
       * @param data_store The data store to copy.
       */
      template<typename DataStore>
      explicit HistoricalDataStore(DataStore data_store);

      explicit HistoricalDataStore(HistoricalDataStore* data_store);

      explicit HistoricalDataStore(
        const std::shared_ptr<HistoricalDataStore>& data_store);

      explicit HistoricalDataStore(
        const std::unique_ptr<HistoricalDataStore>& data_store);

      /**
       * Loads SecurityInfo objects that match a query.
       * @param query The query to load.
       * @return The list of all SecurityInfo objects that match the
       *         <i>query</i>.
       */
      std::vector<SecurityInfo> load_security_info(
        const SecurityInfoQuery& query);

      /**
       * Stores a SecurityInfo.
       * @param info The SecurityInfo to store.
       */
      void store(const SecurityInfo& info);

      /**
       * Executes a search query over a Market's OrderImbalances.
       * @param query The search query to execute.
       * @return The list of the OrderImbalances that satisfy the search
       *         <i>query</i>.
       */
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);

      /**
       * Stores a SequencedVenueOrderImbalance.
       * @param imbalance The SequencedVenueOrderImbalance to store.
       */
      void store(const SequencedVenueOrderImbalance& imbalance);

      /**
       * Stores a list of SequencedVenueOrderImbalance.
       * @param imbalances The list of SequencedVenueOrderImbalances to store.
       */
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);

      /**
       * Executes a search query over a Security's BboQuotes.
       * @param query The search query to execute.
       * @return The list of BboQuotes that satisfy the search <i>query</i>.
       */
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const SecurityMarketDataQuery& query);

      /**
       * Stores a SequencedSecurityBboQuote.
       * @param quote The SequencedSecurityBboQuote to store.
       */
      void store(const SequencedSecurityBboQuote& quote);

      /**
       * Stores a list of SequencedSecurityBboQuotes.
       * @param quotes The list of SequencedSecurityBboQuotes to store.
       */
      void store(const std::vector<SequencedSecurityBboQuote>& quotes);

      /**
       * Executes a search query over a Security's BookQuotes.
       * @param query The search query to execute.
       * @return The list of BookQuotes that satisfy the search <i>query</i>.
       */
      std::vector<SequencedBookQuote> load_book_quotes(
        const SecurityMarketDataQuery& query);

      /**
       * Stores a SequencedSecurityBookQuote.
       * @param quote The SequencedSecurityBookQuote to store.
       */
      void store(const SequencedSecurityBookQuote& quote);

      /**
       * Stores a list of SequencedSecurityBookQuotes.
       * @param quotes The list of SequencedSecurityBookQuotes to store.
       */
      void store(const std::vector<SequencedSecurityBookQuote>& quotes);

      /**
       * Executes a search query over a Security's TimeAndSales.
       * @param query The search query to execute.
       * @return The list of TimeAndSales that satisfy the search <i>query</i>.
       */
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const SecurityMarketDataQuery& query);

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
        virtual void store(const SecurityInfo& info) = 0;
        virtual std::vector<SequencedOrderImbalance> load_order_imbalances(
          const VenueMarketDataQuery& query) = 0;
        virtual void store(const SequencedVenueOrderImbalance& imbalance) = 0;
        virtual void store(
          const std::vector<SequencedVenueOrderImbalance>& imbalances) = 0;
        virtual std::vector<SequencedBboQuote> load_bbo_quotes(
          const SecurityMarketDataQuery& query) = 0;
        virtual void store(const SequencedSecurityBboQuote& quote) = 0;
        virtual void store(
          const std::vector<SequencedSecurityBboQuote>& quotes) = 0;
        virtual std::vector<SequencedBookQuote> load_book_quotes(
          const SecurityMarketDataQuery& query) = 0;
        virtual void store(const SequencedSecurityBookQuote& quote) = 0;
        virtual void store(
          const std::vector<SequencedSecurityBookQuote>& quotes) = 0;
        virtual std::vector<SequencedTimeAndSale> load_time_and_sales(
          const SecurityMarketDataQuery& query) = 0;
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
        void store(const SecurityInfo& info) override;
        std::vector<SequencedOrderImbalance> load_order_imbalances(
          const VenueMarketDataQuery& query) override;
        void store(const SequencedVenueOrderImbalance& imbalance) override;
        void store(
          const std::vector<SequencedVenueOrderImbalance>& imbalances) override;
        std::vector<SequencedBboQuote> load_bbo_quotes(
          const SecurityMarketDataQuery& query) override;
        void store(const SequencedSecurityBboQuote& quote) override;
        void store(
          const std::vector<SequencedSecurityBboQuote>& quotes) override;
        std::vector<SequencedBookQuote> load_book_quotes(
          const SecurityMarketDataQuery& query) override;
        void store(const SequencedSecurityBookQuote& quote) override;
        void store(
          const std::vector<SequencedSecurityBookQuote>& quotes) override;
        std::vector<SequencedTimeAndSale> load_time_and_sales(
          const SecurityMarketDataQuery& query) override;
        void store(const SequencedSecurityTimeAndSale& time_and_sale) override;
        void store(const std::vector<SequencedSecurityTimeAndSale>&
          time_and_sales) override;
        void close() override;
      };
      std::shared_ptr<VirtualHistoricalDataStore> m_data_store;
  };

  /** Checks if a type implements a HistoricalDataStore. */
  template<typename T>
  concept IsHistoricalDataStore = std::constructible_from<
    HistoricalDataStore, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  /**
   * Provides a generic means of loading data from a HistoricalDataStore.
   * @param <T> The type of market data to query.
   * @param data_store The HistoricalDataStore to load from.
   * @param query The search query to execute.
   * @return The list of values that satisfy the search <i>query</i>.
   */
  template<typename T, IsHistoricalDataStore D>
  auto load(D& data_store, const VenueMarketDataQuery& query) {
    if constexpr(std::is_same_v<T, OrderImbalance> ||
        std::is_same_v<T, SequencedOrderImbalance>) {
      return data_store.load_order_imbalances(query);
    } else {
      static_assert("Invalid market data type.");
    }
  }

  /**
   * Provides a generic means of loading data from a HistoricalDataStore.
   * @param <T> The type of market data to query.
   * @param data_store The HistoricalDataStore to load from.
   * @param query The search query to execute.
   * @return The list of values that satisfy the search <i>query</i>.
   */
  template<typename T, IsHistoricalDataStore D>
  auto load(D& data_store, const SecurityMarketDataQuery& query) {
    if constexpr(
        std::is_same_v<T, BboQuote> || std::is_same_v<T, SequencedBboQuote>) {
      return data_store.load_bbo_quotes(query);
    } else if constexpr(
        std::is_same_v<T, BookQuote> || std::is_same_v<T, SequencedBookQuote>) {
      return data_store.load_book_quotes(query);
    } else if constexpr(std::is_same_v<T, TimeAndSale> ||
        std::is_same_v<T, SequencedTimeAndSale>) {
      return data_store.load_time_and_sales(query);
    } else {
      static_assert("Invalid market data type.");
    }
  }

  template<typename T, typename... Args>
  HistoricalDataStore::HistoricalDataStore(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(std::make_shared<WrappedHistoricalDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<typename DataStore>
  HistoricalDataStore::HistoricalDataStore(DataStore data_store)
    : HistoricalDataStore(
        std::in_place_type<DataStore>, std::move(data_store)) {}

  inline HistoricalDataStore::HistoricalDataStore(
    HistoricalDataStore* data_store)
    : HistoricalDataStore(*data_store) {}

  inline HistoricalDataStore::HistoricalDataStore(
    const std::shared_ptr<HistoricalDataStore>& data_store)
    : HistoricalDataStore(*data_store) {}

  inline HistoricalDataStore::HistoricalDataStore(
    const std::unique_ptr<HistoricalDataStore>& data_store)
    : HistoricalDataStore(*data_store) {}

  inline std::vector<SecurityInfo> HistoricalDataStore::load_security_info(
      const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  inline void HistoricalDataStore::store(const SecurityInfo& info) {
    m_data_store->store(info);
  }

  inline std::vector<SequencedOrderImbalance>
      HistoricalDataStore::load_order_imbalances(
        const VenueMarketDataQuery& query) {
    return m_data_store->load_order_imbalances(query);
  }

  inline void HistoricalDataStore::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_data_store->store(imbalance);
  }

  inline void HistoricalDataStore::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_data_store->store(imbalances);
  }

  inline std::vector<SequencedBboQuote> HistoricalDataStore::load_bbo_quotes(
      const SecurityMarketDataQuery& query) {
    return m_data_store->load_bbo_quotes(query);
  }

  inline void HistoricalDataStore::store(
      const SequencedSecurityBboQuote& quote) {
    m_data_store->store(quote);
  }

  inline void HistoricalDataStore::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_data_store->store(quotes);
  }

  inline std::vector<SequencedBookQuote> HistoricalDataStore::
      load_book_quotes(const SecurityMarketDataQuery& query) {
    return m_data_store->load_book_quotes(query);
  }

  inline void HistoricalDataStore::store(
      const SequencedSecurityBookQuote& quote) {
    m_data_store->store(quote);
  }

  inline void HistoricalDataStore::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_data_store->store(quotes);
  }

  inline std::vector<SequencedTimeAndSale>
      HistoricalDataStore::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_time_and_sales(query);
  }

  inline void HistoricalDataStore::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_data_store->store(time_and_sale);
  }

  inline void HistoricalDataStore::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_data_store->store(time_and_sales);
  }

  inline void HistoricalDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  HistoricalDataStore::WrappedHistoricalDataStore<D>::
    WrappedHistoricalDataStore(Args&&... args)
    : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  std::vector<SecurityInfo>
      HistoricalDataStore::WrappedHistoricalDataStore<D>::load_security_info(
        const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SecurityInfo& info) {
    m_data_store->store(info);
  }

  template<typename D>
  std::vector<SequencedOrderImbalance>
      HistoricalDataStore::WrappedHistoricalDataStore<D>::
        load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_data_store->load_order_imbalances(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_data_store->store(imbalance);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_data_store->store(imbalances);
  }

  template<typename D>
  std::vector<SequencedBboQuote> HistoricalDataStore::
      WrappedHistoricalDataStore<D>::load_bbo_quotes(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_bbo_quotes(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SequencedSecurityBboQuote& quote) {
    m_data_store->store(quote);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_data_store->store(quotes);
  }

  template<typename D>
  std::vector<SequencedBookQuote> HistoricalDataStore::
      WrappedHistoricalDataStore<D>::load_book_quotes(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_book_quotes(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SequencedSecurityBookQuote& quote) {
    m_data_store->store(quote);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_data_store->store(quotes);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale> HistoricalDataStore::
      WrappedHistoricalDataStore<D>::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return m_data_store->load_time_and_sales(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_data_store->store(time_and_sale);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_data_store->store(time_and_sales);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
