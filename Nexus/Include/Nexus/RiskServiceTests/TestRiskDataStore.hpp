#ifndef NEXUS_TEST_RISK_DATA_STORE_HPP
#define NEXUS_TEST_RISK_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Routines/Async.hpp>
#include <boost/variant.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTests.hpp"

namespace Nexus::RiskService::Tests {

  /** Implements a RiskDataStore for testing purposes. */
  class TestRiskDataStore {
    public:

      /** Stores a call to the Close method. */
      struct CloseOperation {

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** Stores a call to the LoadInventorySnapshot method. */
      struct LoadInventorySnapshotOperation {

        /** Stores the account argument. */
        const Beam::ServiceLocator::DirectoryEntry* m_account;

        /** The snapshot to return to the caller. */
        Beam::Routines::Eval<InventorySnapshot> m_result;
      };

      /** Stores a call to the Store method. */
      struct StoreInventorySnapshotOperation {

        /** Stores the account argument. */
        const Beam::ServiceLocator::DirectoryEntry* m_account;

        /** Stores the snapshot argument. */
        const InventorySnapshot* m_snapshot;

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** A variant over all method calls. */
      using Operation = boost::variant<CloseOperation,
        LoadInventorySnapshotOperation, StoreInventorySnapshotOperation>;

      /** Constructs a TestRiskDataStore. */
      TestRiskDataStore() = default;

      ~TestRiskDataStore();

      /** Returns an object publishing pending operations. */
      const Beam::Publisher<std::shared_ptr<Operation>>& GetPublisher() const;

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot);

      void Close();

    private:
      Beam::QueueWriterPublisher<std::shared_ptr<Operation>> m_publisher;
      Beam::IO::OpenState m_openState;
  };

  inline TestRiskDataStore::~TestRiskDataStore() {
    m_openState.Close();
  }

  inline const Beam::Publisher<std::shared_ptr<TestRiskDataStore::Operation>>&
      TestRiskDataStore::GetPublisher() const {
    return m_publisher;
  }

  inline InventorySnapshot TestRiskDataStore::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<InventorySnapshot>();
    auto operation = std::make_shared<Operation>(
      LoadInventorySnapshotOperation{&account, result.GetEval()});
    m_publisher.Push(operation);
    return result.Get();
  }

  inline void TestRiskDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      StoreInventorySnapshotOperation{&account, &snapshot, result.GetEval()});
    m_publisher.Push(operation);
    result.Get();
  }

  inline void TestRiskDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      CloseOperation{result.GetEval()});
    m_publisher.Push(operation);
    result.Get();
  }
}

#endif
