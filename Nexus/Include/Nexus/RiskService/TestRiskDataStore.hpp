#ifndef NEXUS_TEST_RISK_DATA_STORE_HPP
#define NEXUS_TEST_RISK_DATA_STORE_HPP
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Routines/Async.hpp>
#include <boost/variant.hpp>
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus::RiskService {

  /** Implements a RiskDataStore for testing purposes. */
  class TestRiskDataStore {
    public:

      /** Stores a call to the Open method. */
      struct OpenOperation {

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

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
      using Operation = boost::variant<OpenOperation, CloseOperation,
        LoadInventorySnapshotOperation, StoreInventorySnapshotOperation>;

      ~TestRiskDataStore();

      /** Returns an object publishing pending operations. */
      const Beam::Publisher<std::shared_ptr<Operation>>& GetPublisher() const;

      InventorySnapshot LoadInventorySnapshot(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const InventorySnapshot& snapshot);

      void Open();

      void Close();

    private:
      Beam::QueueWriterPublisher<std::shared_ptr<Operation>> m_publisher;
      Beam::IO::OpenState m_openState;
  };

  /**
   * Handles opening a TestRiskDataStore.
   * @param dataStore The TestDataStore to open.
   */
  inline void Open(TestRiskDataStore& dataStore) {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto openReceiver = Beam::Routines::Async<void>();
    Beam::Routines::Spawn([&] {
      dataStore.Open();
      openReceiver.GetEval().SetResult();
    });
    while(true) {
      auto operation = operations->Pop();
      if(auto openOperation =
          boost::get<TestRiskDataStore::OpenOperation>(&*operation)) {
        openOperation->m_result.SetResult();
        openReceiver.Get();
        break;
      }
    }
  }

  inline TestRiskDataStore::~TestRiskDataStore() {
    m_openState.SetClosed();
  }

  inline const Beam::Publisher<std::shared_ptr<TestRiskDataStore::Operation>>&
      TestRiskDataStore::GetPublisher() const {
    return m_publisher;
  }

  inline InventorySnapshot TestRiskDataStore::LoadInventorySnapshot(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
    }
    auto result = Beam::Routines::Async<InventorySnapshot>();
    auto operation = std::make_shared<Operation>(
      LoadInventorySnapshotOperation{&account, result.GetEval()});
    m_publisher.Push(operation);
    return result.Get();
  }

  inline void TestRiskDataStore::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const InventorySnapshot& snapshot) {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
    }
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      StoreInventorySnapshotOperation{&account, &snapshot, result.GetEval()});
    m_publisher.Push(operation);
    result.Get();
  }

  inline void TestRiskDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      OpenOperation{result.GetEval()});
    m_publisher.Push(operation);
    try {
      result.Get();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      m_openState.SetClosed();
    }
    m_openState.SetOpen();
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
