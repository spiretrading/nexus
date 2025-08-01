#ifndef NEXUS_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_ORDER_EXECUTION_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"

namespace Nexus::OrderExecutionService {

  /** Provides a generic interface over an arbitrary OrderExecutionDataStore. */
  class OrderExecutionDataStore {
    public:

      /**
       * Constructs an OrderExecutionDataStore of a specified type using
       * emplacement.
       * @param <T> The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<typename T, typename... Args>
      explicit OrderExecutionDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an OrderExecutionDataStore by copying an existing data
       * store.
       * @param data_store The data store to copy.
       */
      template<typename DataStore>
      explicit OrderExecutionDataStore(DataStore data_store);

      explicit OrderExecutionDataStore(
        OrderExecutionDataStore* data_store);

      explicit OrderExecutionDataStore(
        const std::shared_ptr<OrderExecutionDataStore>& data_store);

      explicit OrderExecutionDataStore(
        const std::unique_ptr<OrderExecutionDataStore>& data_store);

      /**
       * Loads an OrderRecord from its id.
       * @param id The Order id to load.
       * @return The OrderRecord with the given <i>id</i> if it exists.
       */
      boost::optional<SequencedAccountOrderRecord>
        load_order_record(OrderId id);

      /**
       * Executes a query to load OrderRecords.
       * @param query The query to execute.
       * @return The list of SequencedOrderRecords satisfying the <i>query</i>.
       */
      std::vector<SequencedOrderRecord>
        load_order_records(const AccountQuery& query);

      /**
       * Stores a SequencedAccountOrderInfo.
       * @param info The SequencedAccountOrderInfo to store.
       */
      void store(const SequencedAccountOrderInfo& info);

      /**
       * Stores a list of SequencedAccountOrderInfo entries.
       * @param info The list of SequencedAccountOrderInfo entries to store.
       */
      void store(const std::vector<SequencedAccountOrderInfo>& info);

      /**
       * Executes a query to load ExecutionReports.
       * @param query The search query to execute.
       * @return The list of SequencedExecutionReports satisfying the
       *         <i>query</i>.
       */
      std::vector<SequencedExecutionReport>
        load_execution_reports(const AccountQuery& query);

      /**
       * Stores a SequencedAccountExecutionReport.
       * @param report The SequencedAccountExecutionReport to store.
       */
      void store(const SequencedAccountExecutionReport& report);

      /**
       * Stores a list of SequencedAccountExecutionReports.
       * @param reports The list of SequencedAccountExecutionReports to store.
       */
      void store(const std::vector<SequencedAccountExecutionReport>& reports);

      void close();

    private:
      struct VirtualOrderExecutionDataStore {
        virtual ~VirtualOrderExecutionDataStore() = default;
        virtual boost::optional<SequencedAccountOrderRecord>
          load_order_record(OrderId) = 0;
        virtual std::vector<SequencedOrderRecord>
          load_order_records(const AccountQuery&) = 0;
        virtual void store(const SequencedAccountOrderInfo&) = 0;
        virtual void store(const std::vector<SequencedAccountOrderInfo>&) = 0;
        virtual std::vector<SequencedExecutionReport>
          load_execution_reports(const AccountQuery&) = 0;
        virtual void store(const SequencedAccountExecutionReport&) = 0;
        virtual void store(
          const std::vector<SequencedAccountExecutionReport>&) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedOrderExecutionDataStore final :
          VirtualOrderExecutionDataStore {
        using OrderExecutionDataStore = D;
        Beam::GetOptionalLocalPtr<OrderExecutionDataStore> m_data_store;

        template<typename... Args>
        WrappedOrderExecutionDataStore(Args&&... args);
        boost::optional<SequencedAccountOrderRecord>
          load_order_record(OrderId) override;
        std::vector<SequencedOrderRecord>
          load_order_records(const AccountQuery&) override;
        void store(const SequencedAccountOrderInfo&) override;
        void store(const std::vector<SequencedAccountOrderInfo>&) override;
        std::vector<SequencedExecutionReport>
          load_execution_reports(const AccountQuery&) override;
        void store(const SequencedAccountExecutionReport&) override;
        void store(
          const std::vector<SequencedAccountExecutionReport>&) override;
        void close() override;
      };
      std::shared_ptr<VirtualOrderExecutionDataStore> m_data_store;
  };

  /** Checks if a type implements an OrderExecutionDataStore. */
  template<typename T>
  concept IsOrderExecutionDataStore = std::constructible_from<
    OrderExecutionDataStore, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  OrderExecutionDataStore::OrderExecutionDataStore(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(std::make_shared<WrappedOrderExecutionDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<typename DataStore>
  OrderExecutionDataStore::OrderExecutionDataStore(DataStore dataStore)
    : OrderExecutionDataStore(
        std::in_place_type<DataStore>, std::move(dataStore)) {}

  inline OrderExecutionDataStore::OrderExecutionDataStore(
    OrderExecutionDataStore* dataStore)
    : OrderExecutionDataStore(*dataStore) {}

  inline OrderExecutionDataStore::OrderExecutionDataStore(
    const std::shared_ptr<OrderExecutionDataStore>& dataStore)
    : OrderExecutionDataStore(*dataStore) {}

  inline OrderExecutionDataStore::OrderExecutionDataStore(
    const std::unique_ptr<OrderExecutionDataStore>& dataStore)
    : OrderExecutionDataStore(*dataStore) {}

  inline boost::optional<SequencedAccountOrderRecord>
      OrderExecutionDataStore::load_order_record(OrderId id) {
    return m_data_store->load_order_record(id);
  }

  inline std::vector<SequencedOrderRecord>
      OrderExecutionDataStore::load_order_records(
        const AccountQuery& query) {
    return m_data_store->load_order_records(query);
  }

  inline void OrderExecutionDataStore::store(
      const SequencedAccountOrderInfo& info) {
    m_data_store->store(info);
  }

  inline void OrderExecutionDataStore::store(
      const std::vector<SequencedAccountOrderInfo>& info) {
    m_data_store->store(info);
  }

  inline std::vector<SequencedExecutionReport>
      OrderExecutionDataStore::load_execution_reports(
        const AccountQuery& query) {
    return m_data_store->load_execution_reports(query);
  }

  inline void OrderExecutionDataStore::store(
      const SequencedAccountExecutionReport& report) {
    m_data_store->store(report);
  }

  inline void OrderExecutionDataStore::store(
      const std::vector<SequencedAccountExecutionReport>& reports) {
    m_data_store->store(reports);
  }

  inline void OrderExecutionDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::
    WrappedOrderExecutionDataStore(Args&&... args)
    : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  boost::optional<SequencedAccountOrderRecord>
      OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::
        load_order_record(OrderId id) {
    return m_data_store->load_order_record(id);
  }

  template<typename D>
  std::vector<SequencedOrderRecord>
      OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::
        load_order_records(const AccountQuery& query) {
    return m_data_store->load_order_records(query);
  }

  template<typename D>
  void OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::store(
      const SequencedAccountOrderInfo& info) {
    m_data_store->store(info);
  }

  template<typename D>
  void OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::store(
      const std::vector<SequencedAccountOrderInfo>& info) {
    m_data_store->store(info);
  }

  template<typename D>
  std::vector<SequencedExecutionReport>
      OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::
        load_execution_reports(const AccountQuery& query) {
    return m_data_store->load_execution_reports(query);
  }

  template<typename D>
  void OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::store(
      const SequencedAccountExecutionReport& report) {
    m_data_store->store(report);
  }

  template<typename D>
  void OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::store(
      const std::vector<SequencedAccountExecutionReport>& reports) {
    m_data_store->store(reports);
  }

  template<typename D>
  void OrderExecutionDataStore::WrappedOrderExecutionDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
