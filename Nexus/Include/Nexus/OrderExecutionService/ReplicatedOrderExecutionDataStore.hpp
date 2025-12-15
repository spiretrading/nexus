#ifndef NEXUS_REPLICATED_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_REPLICATED_ORDER_EXECUTION_DATA_STORE_HPP
#include <atomic>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"

namespace Nexus {

  /** Duplicates an OrderExecutionDataStore across multiple instances. */
  class ReplicatedOrderExecutionDataStore {
    public:

      /**
       * Constructs an empty ReplicatedOrderExecutionDataStore.
       * @param primary_data_store The primary data store to access.
       * @param duplicate_data_stores The data stores to replicate the primary
       *        to.
       */
      ReplicatedOrderExecutionDataStore(
        OrderExecutionDataStore primary_data_store,
        std::vector<OrderExecutionDataStore> duplicate_data_stores);

      ~ReplicatedOrderExecutionDataStore();

      boost::optional<SequencedAccountOrderRecord>
        load_order_record(OrderId id);
      std::vector<SequencedOrderRecord>
        load_order_records(const AccountQuery& query);
      void store(const SequencedAccountOrderInfo& info);
      void store(const std::vector<SequencedAccountOrderInfo>& info);
      std::vector<SequencedExecutionReport>
        load_execution_reports(const AccountQuery& query);
      void store(const SequencedAccountExecutionReport& report);
      void store(const std::vector<SequencedAccountExecutionReport>& reports);
      void close();

    private:
      OrderExecutionDataStore m_primary_data_store;
      std::vector<OrderExecutionDataStore> m_duplicate_data_stores;
      std::atomic_size_t m_next_data_store;

      ReplicatedOrderExecutionDataStore(
        const ReplicatedOrderExecutionDataStore&) = delete;
      ReplicatedOrderExecutionDataStore& operator =(
        const ReplicatedOrderExecutionDataStore&) = delete;
  };

  inline ReplicatedOrderExecutionDataStore::ReplicatedOrderExecutionDataStore(
    OrderExecutionDataStore primary_data_store,
    std::vector<OrderExecutionDataStore> duplicate_data_stores)
    : m_primary_data_store(std::move(primary_data_store)),
      m_duplicate_data_stores(std::move(duplicate_data_stores)),
      m_next_data_store(0) {}

  inline ReplicatedOrderExecutionDataStore::
      ~ReplicatedOrderExecutionDataStore() {
    close();
  }

  inline boost::optional<SequencedAccountOrderRecord>
      ReplicatedOrderExecutionDataStore::load_order_record(OrderId id) {
    if(m_duplicate_data_stores.empty()) {
      return m_primary_data_store.load_order_record(id);
    }
    auto index = ++m_next_data_store;
    index %= m_duplicate_data_stores.size();
    return m_duplicate_data_stores[index].load_order_record(id);
  }

  inline std::vector<SequencedOrderRecord>
      ReplicatedOrderExecutionDataStore::load_order_records(
      const AccountQuery& query) {
    if(m_duplicate_data_stores.empty()) {
      return m_primary_data_store.load_order_records(query);
    }
    auto index = ++m_next_data_store;
    index %= m_duplicate_data_stores.size();
    return m_duplicate_data_stores[index].load_order_records(query);
  }

  inline void ReplicatedOrderExecutionDataStore::store(
      const SequencedAccountOrderInfo& info) {
    m_primary_data_store.store(info);
    for(auto& data_store : m_duplicate_data_stores) {
      data_store.store(info);
    }
  }

  inline void ReplicatedOrderExecutionDataStore::store(
      const std::vector<SequencedAccountOrderInfo>& info) {
    m_primary_data_store.store(info);
    for(auto& data_store : m_duplicate_data_stores) {
      data_store.store(info);
    }
  }

  inline std::vector<SequencedExecutionReport>
      ReplicatedOrderExecutionDataStore::load_execution_reports(
      const AccountQuery& query) {
    if(m_duplicate_data_stores.empty()) {
      return m_primary_data_store.load_execution_reports(query);
    }
    auto index = ++m_next_data_store;
    index %= m_duplicate_data_stores.size();
    return m_duplicate_data_stores[index].load_execution_reports(query);
  }

  inline void ReplicatedOrderExecutionDataStore::store(
      const SequencedAccountExecutionReport& report) {
    m_primary_data_store.store(report);
    for(auto& data_store : m_duplicate_data_stores) {
      data_store.store(report);
    }
  }

  inline void ReplicatedOrderExecutionDataStore::store(
      const std::vector<SequencedAccountExecutionReport>& reports) {
    m_primary_data_store.store(reports);
    for(auto& data_store : m_duplicate_data_stores) {
      data_store.store(reports);
    }
  }

  inline void ReplicatedOrderExecutionDataStore::close() {
    for(auto& data_store : m_duplicate_data_stores) {
      data_store.close();
    }
    m_primary_data_store.close();
  }
}

#endif
