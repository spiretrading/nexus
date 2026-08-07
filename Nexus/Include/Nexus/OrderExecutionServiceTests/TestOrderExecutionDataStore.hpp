#ifndef NEXUS_TEST_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_TEST_ORDER_EXECUTION_DATA_STORE_HPP
#include <memory>
#include <variant>
#include <vector>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Accounting/InventorySnapshot.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"

namespace Nexus::Tests {

  /** Implements an OrderExecutionDataStore for testing purposes. */
  class TestOrderExecutionDataStore {
    public:

      /** Records a call to load_order_record. */
      struct LoadOrderRecordOperation {

        /** The id of the Order to load. */
        OrderId m_id;

        /** The value to return. */
        Beam::Tests::ServiceResult<
          boost::optional<SequencedAccountOrderRecord>> m_result;
      };

      /** Records a call to load_order_records. */
      struct LoadOrderRecordsOperation {

        /** The query to execute. */
        AccountQuery m_query;

        /** The value to return. */
        Beam::Tests::ServiceResult<std::vector<SequencedOrderRecord>> m_result;
      };

      /** Records a call to store a SequencedAccountOrderInfo. */
      struct StoreOrderInfoOperation {

        /** The OrderInfo passed. */
        SequencedAccountOrderInfo m_info;

        /** The value to return. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to store a list of SequencedAccountOrderInfo. */
      struct StoreOrderInfoListOperation {

        /** The list of OrderInfo passed. */
        std::vector<SequencedAccountOrderInfo> m_info;

        /** The value to return. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to load_execution_reports. */
      struct LoadExecutionReportsOperation {

        /** The query to execute. */
        AccountQuery m_query;

        /** The value to return. */
        Beam::Tests::ServiceResult<std::vector<SequencedExecutionReport>>
          m_result;
      };

      /** Records a call to store a SequencedAccountExecutionReport. */
      struct StoreExecutionReportOperation {

        /** The ExecutionReport passed. */
        SequencedAccountExecutionReport m_report;

        /** The value to return. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to store a list of SequencedAccountExecutionReports. */
      struct StoreExecutionReportListOperation {

        /** The list of ExecutionReports passed. */
        std::vector<SequencedAccountExecutionReport> m_reports;

        /** The value to return. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to load_inventory_snapshot. */
      struct LoadInventorySnapshotOperation {

        /** The account whose snapshot is to be loaded. */
        Beam::DirectoryEntry m_account;

        /** The value to return. */
        Beam::Tests::ServiceResult<InventorySnapshot> m_result;
      };

      /** Records a call to store an account's InventorySnapshot. */
      struct StoreInventorySnapshotOperation {

        /** The account whose snapshot is being stored. */
        Beam::DirectoryEntry m_account;

        /** The snapshot passed. */
        InventorySnapshot m_snapshot;

        /** The value to return. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** A variant covering all possible operations. */
      using Operation = std::variant<LoadOrderRecordOperation,
        LoadOrderRecordsOperation, StoreOrderInfoOperation,
        StoreOrderInfoListOperation, LoadExecutionReportsOperation,
        StoreExecutionReportOperation, StoreExecutionReportListOperation,
        LoadInventorySnapshotOperation, StoreInventorySnapshotOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestOrderExecutionDataStore.
       * @param operations The queue to push all operations on.
       */
      explicit TestOrderExecutionDataStore(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations)
          noexcept;

      ~TestOrderExecutionDataStore();

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
      InventorySnapshot load_inventory_snapshot(
        const Beam::DirectoryEntry& account);
      void store(const Beam::DirectoryEntry& account,
        const InventorySnapshot& snapshot);
      void close();

    private:
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_operations;

      TestOrderExecutionDataStore(const TestOrderExecutionDataStore&) = delete;
      TestOrderExecutionDataStore& operator =(
        const TestOrderExecutionDataStore&) = delete;
  };

  inline TestOrderExecutionDataStore::TestOrderExecutionDataStore(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestOrderExecutionDataStore::~TestOrderExecutionDataStore() {
    close();
  }

  inline boost::optional<SequencedAccountOrderRecord>
      TestOrderExecutionDataStore::load_order_record(OrderId id) {
    return m_operations.append_result<LoadOrderRecordOperation,
      boost::optional<SequencedAccountOrderRecord>>(id);
  }

  inline std::vector<SequencedOrderRecord>
      TestOrderExecutionDataStore::load_order_records(
        const AccountQuery& query) {
    return m_operations.append_result<
      LoadOrderRecordsOperation, std::vector<SequencedOrderRecord>>(query);
  }

  inline void TestOrderExecutionDataStore::store(
      const SequencedAccountOrderInfo& info) {
    return m_operations.append_result<StoreOrderInfoOperation, void>(info);
  }

  inline void TestOrderExecutionDataStore::store(
      const std::vector<SequencedAccountOrderInfo>& info) {
    return m_operations.append_result<StoreOrderInfoListOperation, void>(info);
  }

  inline std::vector<SequencedExecutionReport>
      TestOrderExecutionDataStore::load_execution_reports(
        const AccountQuery& query) {
    return m_operations.append_result<LoadExecutionReportsOperation,
      std::vector<SequencedExecutionReport>>(query);
  }

  inline void TestOrderExecutionDataStore::store(
      const SequencedAccountExecutionReport& report) {
    return m_operations.append_result<StoreExecutionReportOperation, void>(
      report);
  }

  inline void TestOrderExecutionDataStore::store(
      const std::vector<SequencedAccountExecutionReport>& reports) {
    return m_operations.append_result<StoreExecutionReportListOperation, void>(
      reports);
  }

  inline InventorySnapshot
      TestOrderExecutionDataStore::load_inventory_snapshot(
        const Beam::DirectoryEntry& account) {
    return m_operations.append_result<
      LoadInventorySnapshotOperation, InventorySnapshot>(account);
  }

  inline void TestOrderExecutionDataStore::store(
      const Beam::DirectoryEntry& account, const InventorySnapshot& snapshot) {
    return m_operations.append_result<StoreInventorySnapshotOperation, void>(
      account, snapshot);
  }

  inline void TestOrderExecutionDataStore::close() {
    m_operations.close();
  }
}

#endif
