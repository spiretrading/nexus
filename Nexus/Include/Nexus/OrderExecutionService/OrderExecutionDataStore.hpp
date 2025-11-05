#ifndef NEXUS_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_ORDER_EXECUTION_DATA_STORE_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/VirtualPtr.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"

namespace Nexus {

  /** Checks if a type implements an OrderExecutionDataStore. */
  template<typename T>
  concept IsOrderExecutionDataStore =
    Beam::IsConnection<T> && requires(T& data_store) {
      { data_store.load_order_record(std::declval<OrderId>()) } ->
          std::same_as<boost::optional<SequencedAccountOrderRecord>>;
      { data_store.load_order_records(std::declval<const AccountQuery&>()) } ->
          std::same_as<std::vector<SequencedOrderRecord>>;
      data_store.store(std::declval<const SequencedAccountOrderInfo&>());
      data_store.store(
        std::declval<const std::vector<SequencedAccountOrderInfo>&>());
      { data_store.load_execution_reports(
          std::declval<const AccountQuery&>()) } ->
            std::same_as<std::vector<SequencedExecutionReport>>;
      data_store.store(std::declval<const SequencedAccountExecutionReport&>());
      data_store.store(
        std::declval<const std::vector<SequencedAccountExecutionReport>&>());
    };

  /** Provides a generic interface over an arbitrary OrderExecutionDataStore. */
  class OrderExecutionDataStore {
    public:

      /**
       * Constructs an OrderExecutionDataStore of a specified type using
       * emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsOrderExecutionDataStore T, typename... Args>
      explicit OrderExecutionDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an OrderExecutionDataStore by referencing an existing data
       * store.
       * @param data_store The data store to reference.
       */
      template<Beam::DisableCopy<OrderExecutionDataStore> T> requires
        IsOrderExecutionDataStore<Beam::dereference_t<T>>
      OrderExecutionDataStore(T&& data_store);

      OrderExecutionDataStore(const OrderExecutionDataStore&) = default;
      OrderExecutionDataStore(OrderExecutionDataStore&&) = default;

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
        Beam::local_ptr_t<OrderExecutionDataStore> m_data_store;

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
      Beam::VirtualPtr<VirtualOrderExecutionDataStore> m_data_store;
  };

  template<IsOrderExecutionDataStore T, typename... Args>
  OrderExecutionDataStore::OrderExecutionDataStore(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(Beam::make_virtual_ptr<WrappedOrderExecutionDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<OrderExecutionDataStore> T> requires
    IsOrderExecutionDataStore<Beam::dereference_t<T>>
  OrderExecutionDataStore::OrderExecutionDataStore(T&& data_store)
    : m_data_store(Beam::make_virtual_ptr<WrappedOrderExecutionDataStore<
        std::remove_cvref_t<T>>>(std::forward<T>(data_store))) {}

  inline boost::optional<SequencedAccountOrderRecord>
      OrderExecutionDataStore::load_order_record(OrderId id) {
    return m_data_store->load_order_record(id);
  }

  inline std::vector<SequencedOrderRecord>
      OrderExecutionDataStore::load_order_records(const AccountQuery& query) {
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
