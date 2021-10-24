#ifndef NEXUS_TELEMETRY_SERVICES_HPP
#define NEXUS_TELEMETRY_SERVICES_HPP
#include <Beam/Json/JsonObject.hpp>
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/TelemetryService/AccountQuery.hpp"
#include "Nexus/TelemetryService/TelemetryEvent.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {
  using AccountTelemetryQueryResult =
    Beam::Queries::QueryResult<SequencedTelemetryEvent>;
  BEAM_DEFINE_RECORD(RecordUpdate, std::string, name, Beam::JsonObject, data);

  BEAM_DEFINE_SERVICES(TelemetryServices,

    /**
     * Queries telemetry for an account.
     * @param query The query to submit.
     * @return The list of TelementryEvent's satisfying the query.
     */
    (QueryAccountTelemetryService,
      "Nexus.TelemetryService.QueryAccountTelemetryService",
      AccountTelemetryQueryResult, AccountQuery, query),

    /**
     * Records a list of telemetry events.
     * @param updates <code>std::vector\<RecordUpdate\></code> The list of
     *        updates to record.
     */
    (RecordService, "Nexus.TelemetryService.RecordService", void,
      std::vector<RecordUpdate>, updates));

  BEAM_DEFINE_MESSAGES(TelemetryMessages,

    /**
     * Sends a query's SequencedAccountTelemetryEvent.
     * @param telemetry_event The query's SequencedAccountTelemetryEvent.
     */
    (TelemetryEventMessage, "Nexus.TelemetryService.TelemetryEventMessage",
      SequencedAccountTelemetryEvent, telemetry_event),

    /**
     * Terminates a previous TelemetryEvent query.
     * @param account The account that was queried.
     * @param id The id of query to end.
     */
    (EndAccountTelemetryEventQueryMessage,
      "Nexus.TelemetryService.EndAccountTelemetryEventQueryMessage",
      Beam::ServiceLocator::DirectoryEntry, account, int, id));

}

#endif
