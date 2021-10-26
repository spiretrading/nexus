#ifndef NEXUS_TEST_TELEMETRY_DATA_STORE_HPP
#define NEXUS_TEST_TELEMETRY_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Routines/Async.hpp>
#include <boost/variant.hpp>
#include "Nexus/TelemetryService/TelemetryDataStore.hpp"
#include "Nexus/TelemetryServiceTests/TelemetryServiceTests.hpp"

namespace Nexus::TelemetryService::Tests {

  /** Implements a TelemetryDataStore for testing purposes. */
  class TestTelemetryDataStore {
    public:

      /** Stores a call to the Close method. */
      struct CloseOperation {

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** Stores a call to the LoadTelemetryEvents method. */
      struct LoadTelemetryEventsOperation {

        /** Stores the query argument. */
        const AccountQuery* m_query;

        /** The snapshot to return to the caller. */
        Beam::Routines::Eval<std::vector<SequencedTelemetryEvent>> m_result;
      };

      /** Stores a call to the Store method for a single TelemetryEvent. */
      struct StoreEventOperation {

        /** Stores the event argument. */
        const SequencedAccountTelemetryEvent* m_event;

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** Stores a call to the Store method for a list of TelemetryEvents. */
      struct StoreEventListOperation {

        /** Stores the events argument. */
        const std::vector<SequencedAccountTelemetryEvent>* m_events;

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** A variant over all method calls. */
      using Operation = boost::variant<CloseOperation,
        LoadTelemetryEventsOperation, StoreEventOperation,
        StoreEventListOperation>;

      /** Constructs a TestTelemetryDataStore. */
      TestTelemetryDataStore() = default;

      ~TestTelemetryDataStore();

      /** Returns an object publishing pending operations. */
      const Beam::Publisher<std::shared_ptr<Operation>>& GetPublisher() const;

      std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
        const AccountQuery& query);

      void Store(const SequencedAccountTelemetryEvent& event);

      void Store(const std::vector<SequencedAccountTelemetryEvent>& events);

      void Close();

    private:
      Beam::QueueWriterPublisher<std::shared_ptr<Operation>> m_publisher;
      Beam::IO::OpenState m_openState;
  };

  inline TestTelemetryDataStore::~TestTelemetryDataStore() {
    m_openState.Close();
  }

  inline const Beam::Publisher<
      std::shared_ptr<TestTelemetryDataStore::Operation>>&
        TestTelemetryDataStore::GetPublisher() const {
    return m_publisher;
  }

  inline std::vector<SequencedTelemetryEvent>
      TestTelemetryDataStore::LoadTelemetryEvents(const AccountQuery& query) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<std::vector<SequencedTelemetryEvent>>();
    auto operation = std::make_shared<Operation>(
      LoadTelemetryEventsOperation(&query, result.GetEval()));
    m_publisher.Push(operation);
    return result.Get();
  }

  inline void TestTelemetryDataStore::Store(
      const SequencedAccountTelemetryEvent& event) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      StoreEventOperation(&event, result.GetEval()));
    m_publisher.Push(operation);
    result.Get();
  }

  void TestTelemetryDataStore::Store(
      const std::vector<SequencedAccountTelemetryEvent>& events) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      StoreEventListOperation(&events, result.GetEval()));
    m_publisher.Push(operation);
    result.Get();
  }

  inline void TestTelemetryDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto result = Beam::Routines::Async<void>();
    auto operation =
      std::make_shared<Operation>(CloseOperation(result.GetEval()));
    m_publisher.Push(operation);
    result.Get();
  }
}

#endif
