#ifndef NEXUS_HISTORICAL_DATA_STORE_HPP
#define NEXUS_HISTORICAL_DATA_STORE_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Nexus/MarketDataService/TickerMarketDataQuery.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

namespace Nexus {

  /** Concept for types that can be used as a historical data store. */
  template<typename T>
  concept IsHistoricalDataStore = Beam::IsConnection<T> && requires(T& store) {
    { store.load_ticker_info(std::declval<const TickerInfoQuery&>()) } ->
        std::same_as<std::vector<TickerInfo>>;
    store.store(std::declval<const TickerInfo&>());
    { store.load_order_imbalances(
        std::declval<const VenueMarketDataQuery&>()) } ->
          std::same_as<std::vector<SequencedOrderImbalance>>;
    store.store(std::declval<const SequencedVenueOrderImbalance&>());
    store.store(
      std::declval<const std::vector<SequencedVenueOrderImbalance>&>());
    { store.load_bbo_quotes(std::declval<const TickerMarketDataQuery&>()) } ->
        std::same_as<std::vector<SequencedBboQuote>>;
    store.store(std::declval<const SequencedTickerBboQuote&>());
    store.store(std::declval<const std::vector<SequencedTickerBboQuote>&>());
    { store.load_book_quotes(std::declval<const TickerMarketDataQuery&>()) } ->
        std::same_as<std::vector<SequencedBookQuote>>;
    store.store(std::declval<const SequencedTickerBookQuote&>());
    store.store(std::declval<const std::vector<SequencedTickerBookQuote>&>());
    { store.load_time_and_sales(
        std::declval<const TickerMarketDataQuery&>()) } ->
          std::same_as<std::vector<SequencedTimeAndSale>>;
    store.store(std::declval<const SequencedTickerTimeAndSale&>());
    store.store(std::declval<const std::vector<SequencedTickerTimeAndSale>&>());
  };

  /** Provides a generic interface over an arbitrary HistoricalDataStore. */
  class HistoricalDataStore {
    public:

      /**
       * Constructs an HistoricalDataStore of a specified type using
       * emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsHistoricalDataStore T, typename... Args>
      explicit HistoricalDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an HistoricalDataStore by referencing an existing data
       * store.
       * @param data_store The data store to reference.
       */
      template<Beam::DisableCopy<HistoricalDataStore> T> requires
        IsHistoricalDataStore<Beam::dereference_t<T>>
      HistoricalDataStore(T&& data_store);

      HistoricalDataStore(const HistoricalDataStore&) = default;
      HistoricalDataStore(HistoricalDataStore&&) = default;

      /**
       * Loads TickerInfo objects that match a query.
       * @param query The query to load.
       * @return The list of all TickerInfo objects that match the
       *         <i>query</i>.
       */
      std::vector<TickerInfo> load_ticker_info(const TickerInfoQuery& query);

      /**
       * Stores a TickerInfo.
       * @param info The TickerInfo to store.
       */
      void store(const TickerInfo& info);

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
       * Executes a search query over a Ticker's BboQuotes.
       * @param query The search query to execute.
       * @return The list of BboQuotes that satisfy the search <i>query</i>.
       */
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const TickerMarketDataQuery& query);

      /**
       * Stores a SequencedTickerBboQuote.
       * @param quote The SequencedTickerBboQuote to store.
       */
      void store(const SequencedTickerBboQuote& quote);

      /**
       * Stores a list of SequencedTickerBboQuotes.
       * @param quotes The list of SequencedTickerBboQuotes to store.
       */
      void store(const std::vector<SequencedTickerBboQuote>& quotes);

      /**
       * Executes a search query over a Ticker's BookQuotes.
       * @param query The search query to execute.
       * @return The list of BookQuotes that satisfy the search <i>query</i>.
       */
      std::vector<SequencedBookQuote> load_book_quotes(
        const TickerMarketDataQuery& query);

      /**
       * Stores a SequencedTickerBookQuote.
       * @param quote The SequencedTickerBookQuote to store.
       */
      void store(const SequencedTickerBookQuote& quote);

      /**
       * Stores a list of SequencedTickerBookQuotes.
       * @param quotes The list of SequencedTickerBookQuotes to store.
       */
      void store(const std::vector<SequencedTickerBookQuote>& quotes);

      /**
       * Executes a search query over a Ticker's TimeAndSales.
       * @param query The search query to execute.
       * @return The list of TimeAndSales that satisfy the search <i>query</i>.
       */
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const TickerMarketDataQuery& query);

      /**
       * Stores a SequencedTickerTimeAndSale.
       * @param time_and_sale The SequencedTickerTimeAndSale to store.
       */
      void store(const SequencedTickerTimeAndSale& time_and_sale);

      /**
       * Stores a list of SequencedTickerTimeAndSales.
       * @param time_and_sales The list of SequencedTickerTimeAndSales to
       *        store.
       */
      void store(const std::vector<SequencedTickerTimeAndSale>& time_and_sales);

      /** Closes the data store. */
      void close();

    private:
      struct VirtualHistoricalDataStore {
        virtual ~VirtualHistoricalDataStore() = default;

        virtual std::vector<TickerInfo> load_ticker_info(
          const TickerInfoQuery&) = 0;
        virtual void store(const TickerInfo&) = 0;
        virtual std::vector<SequencedOrderImbalance> load_order_imbalances(
          const VenueMarketDataQuery&) = 0;
        virtual void store(const SequencedVenueOrderImbalance&) = 0;
        virtual void store(
          const std::vector<SequencedVenueOrderImbalance>&) = 0;
        virtual std::vector<SequencedBboQuote> load_bbo_quotes(
          const TickerMarketDataQuery&) = 0;
        virtual void store(const SequencedTickerBboQuote&) = 0;
        virtual void store(const std::vector<SequencedTickerBboQuote>&) = 0;
        virtual std::vector<SequencedBookQuote> load_book_quotes(
          const TickerMarketDataQuery&) = 0;
        virtual void store(const SequencedTickerBookQuote&) = 0;
        virtual void store(const std::vector<SequencedTickerBookQuote>&) = 0;
        virtual std::vector<SequencedTimeAndSale> load_time_and_sales(
          const TickerMarketDataQuery&) = 0;
        virtual void store(const SequencedTickerTimeAndSale&) = 0;
        virtual void store(const std::vector<SequencedTickerTimeAndSale>&) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedHistoricalDataStore final : VirtualHistoricalDataStore {
        using DataStore = D;
        Beam::local_ptr_t<DataStore> m_data_store;

        template<typename... Args>
        WrappedHistoricalDataStore(Args&&... args);

        std::vector<TickerInfo> load_ticker_info(
          const TickerInfoQuery& query) override;
        void store(const TickerInfo& info) override;
        std::vector<SequencedOrderImbalance> load_order_imbalances(
          const VenueMarketDataQuery& query) override;
        void store(const SequencedVenueOrderImbalance& imbalance) override;
        void store(
          const std::vector<SequencedVenueOrderImbalance>& imbalances) override;
        std::vector<SequencedBboQuote> load_bbo_quotes(
          const TickerMarketDataQuery& query) override;
        void store(const SequencedTickerBboQuote& quote) override;
        void store(const std::vector<SequencedTickerBboQuote>& quotes) override;
        std::vector<SequencedBookQuote> load_book_quotes(
          const TickerMarketDataQuery& query) override;
        void store(const SequencedTickerBookQuote& quote) override;
        void store(
          const std::vector<SequencedTickerBookQuote>& quotes) override;
        std::vector<SequencedTimeAndSale> load_time_and_sales(
          const TickerMarketDataQuery& query) override;
        void store(const SequencedTickerTimeAndSale& time_and_sale) override;
        void store(const std::vector<SequencedTickerTimeAndSale>&
          time_and_sales) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualHistoricalDataStore> m_data_store;
  };

  /**
   * Provides a generic means of loading data from a HistoricalDataStore.
   * @tparam T The type of market data to query.
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
   * @tparam T The type of market data to query.
   * @param data_store The HistoricalDataStore to load from.
   * @param query The search query to execute.
   * @return The list of values that satisfy the search <i>query</i>.
   */
  template<typename T, IsHistoricalDataStore D>
  auto load(D& data_store, const TickerMarketDataQuery& query) {
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

  template<IsHistoricalDataStore T, typename... Args>
  HistoricalDataStore::HistoricalDataStore(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(Beam::make_virtual_ptr<WrappedHistoricalDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<HistoricalDataStore> T> requires
    IsHistoricalDataStore<Beam::dereference_t<T>>
  HistoricalDataStore::HistoricalDataStore(T&& data_store)
    : m_data_store(Beam::make_virtual_ptr<WrappedHistoricalDataStore<
        std::remove_cvref_t<T>>>(std::forward<T>(data_store))) {}

  inline std::vector<TickerInfo> HistoricalDataStore::load_ticker_info(
      const TickerInfoQuery& query) {
    return m_data_store->load_ticker_info(query);
  }

  inline void HistoricalDataStore::store(const TickerInfo& info) {
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
      const TickerMarketDataQuery& query) {
    return m_data_store->load_bbo_quotes(query);
  }

  inline void HistoricalDataStore::store(const SequencedTickerBboQuote& quote) {
    m_data_store->store(quote);
  }

  inline void HistoricalDataStore::store(
      const std::vector<SequencedTickerBboQuote>& quotes) {
    m_data_store->store(quotes);
  }

  inline std::vector<SequencedBookQuote> HistoricalDataStore::
      load_book_quotes(const TickerMarketDataQuery& query) {
    return m_data_store->load_book_quotes(query);
  }

  inline void HistoricalDataStore::store(
      const SequencedTickerBookQuote& quote) {
    m_data_store->store(quote);
  }

  inline void HistoricalDataStore::store(
      const std::vector<SequencedTickerBookQuote>& quotes) {
    m_data_store->store(quotes);
  }

  inline std::vector<SequencedTimeAndSale>
      HistoricalDataStore::load_time_and_sales(
        const TickerMarketDataQuery& query) {
    return m_data_store->load_time_and_sales(query);
  }

  inline void HistoricalDataStore::store(
      const SequencedTickerTimeAndSale& time_and_sale) {
    m_data_store->store(time_and_sale);
  }

  inline void HistoricalDataStore::store(
      const std::vector<SequencedTickerTimeAndSale>& time_and_sales) {
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
  std::vector<TickerInfo>
      HistoricalDataStore::WrappedHistoricalDataStore<D>::load_ticker_info(
        const TickerInfoQuery& query) {
    return m_data_store->load_ticker_info(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const TickerInfo& info) {
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
        const TickerMarketDataQuery& query) {
    return m_data_store->load_bbo_quotes(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SequencedTickerBboQuote& quote) {
    m_data_store->store(quote);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBboQuote>& quotes) {
    m_data_store->store(quotes);
  }

  template<typename D>
  std::vector<SequencedBookQuote> HistoricalDataStore::
      WrappedHistoricalDataStore<D>::load_book_quotes(
        const TickerMarketDataQuery& query) {
    return m_data_store->load_book_quotes(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SequencedTickerBookQuote& quote) {
    m_data_store->store(quote);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerBookQuote>& quotes) {
    m_data_store->store(quotes);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale> HistoricalDataStore::
      WrappedHistoricalDataStore<D>::load_time_and_sales(
        const TickerMarketDataQuery& query) {
    return m_data_store->load_time_and_sales(query);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const SequencedTickerTimeAndSale& time_and_sale) {
    m_data_store->store(time_and_sale);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::store(
      const std::vector<SequencedTickerTimeAndSale>& time_and_sales) {
    m_data_store->store(time_and_sales);
  }

  template<typename D>
  void HistoricalDataStore::WrappedHistoricalDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
