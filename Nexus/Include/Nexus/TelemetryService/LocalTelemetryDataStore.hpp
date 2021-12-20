#ifndef NEXUS_LOCAL_TELEMETRY_DATA_STORE_HPP
#define NEXUS_LOCAL_TELEMETRY_DATA_STORE_HPP
#include <Beam/Queries/LocalDataStore.hpp>
#include "Nexus/TelemetryService/TelemetryDataStore.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::TelemetryService {

  /** Stores an in memory database of telemetry data. */
  class LocalTelemetryDataStore {
    public:

      /** Constructs an empty LocalTelemetryDataStore. */
      LocalTelemetryDataStore() = default;

      /** Returns all stored TelemetryEvents. */
      std::vector<SequencedAccountTelemetryEvent> LoadTelemetryEvents() const;

      std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
        const AccountQuery& query);

      void Store(const SequencedAccountTelemetryEvent& event);

      void Store(const std::vector<SequencedAccountTelemetryEvent>& events);

      void Close();

    private:
      template<typename T>
      using DataStore = Beam::Queries::LocalDataStore<AccountQuery, T,
        Queries::EvaluatorTranslator>;
      DataStore<TelemetryEvent> m_telemetryEventDataStore;
  };

  inline std::vector<SequencedAccountTelemetryEvent>
      LocalTelemetryDataStore::LoadTelemetryEvents() const {
    return m_telemetryEventDataStore.LoadAll();
  }

  inline std::vector<SequencedTelemetryEvent>
      LocalTelemetryDataStore::LoadTelemetryEvents(const AccountQuery& query) {
    return m_telemetryEventDataStore.Load(query);
  }

  inline void LocalTelemetryDataStore::Store(
      const SequencedAccountTelemetryEvent& event) {
    m_telemetryEventDataStore.Store(event);
  }

  inline void LocalTelemetryDataStore::Store(
      const std::vector<SequencedAccountTelemetryEvent>& events) {
    m_telemetryEventDataStore.Store(events);
  }

  inline void LocalTelemetryDataStore::Close() {}
}

#endif
