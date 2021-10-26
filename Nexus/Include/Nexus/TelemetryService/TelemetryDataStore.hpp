#ifndef NEXUS_TELEMETRY_SERVICE_TELEMETRY_DATA_STORE_HPP
#define NEXUS_TELEMETRY_SERVICE_TELEMETRY_DATA_STORE_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Utilities/Concept.hpp>
#include "Nexus/TelemetryService/AccountQuery.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Concept for the type of data store used by the telemetry service. */
  struct TelemetryDataStore : Beam::Concept<TelemetryDataStore> {
    public:

      /**
       * Loads SequencedTelemetryEvents matching a query.
       * @param query The query used to load events.
       * @return The list of all events matching the given <i>query</i>.
       */
      std::vector<SequencedTelemetryEvent> LoadTelemetryEvents(
        const AccountQuery& query);

      /**
       * Stores a SequencedAccountTelemetryEvent.
       * @param event The event to store.
       */
      void Store(const SequencedAccountTelemetryEvent& event);

      /**
       * Stores a list of SequencedAccountTelemetryEvent.
       * @param events The list of events to store.
       */
      void Store(const std::vector<SequencedAccountTelemetryEvent>& events);

      void Close();
  };
}

#endif
