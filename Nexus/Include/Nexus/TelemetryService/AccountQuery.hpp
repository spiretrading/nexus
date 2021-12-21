#ifndef NEXUS_TELEMETRY_SERVICE_ACCOUNT_QUERY_HPP
#define NEXUS_TELEMETRY_SERVICE_ACCOUNT_QUERY_HPP
#include <Beam/Queries/BasicQuery.hpp>
#include <Beam/Queries/IndexedValue.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/TelemetryService/TelemetryEvent.hpp"

namespace Nexus::TelemetryService {
  using SequencedTelemetryEvent = Beam::Queries::SequencedValue<TelemetryEvent>;
  using AccountTelemetryEvent = Beam::Queries::IndexedValue<TelemetryEvent,
    Beam::ServiceLocator::DirectoryEntry>;
  using SequencedAccountTelemetryEvent =
    Beam::Queries::SequencedValue<AccountTelemetryEvent>;

  /** Defines the type of query used to receive telemetry events submissions. */
  using AccountQuery =
    Beam::Queries::BasicQuery<Beam::ServiceLocator::DirectoryEntry>;
}

namespace Beam::Queries {
  template<>
  struct TimestampAccessor<Nexus::TelemetryService::TelemetryEvent> {
    const boost::posix_time::ptime& operator ()(
        const Nexus::TelemetryService::TelemetryEvent& value) const {
      return value.m_timestamp;
    }

    boost::posix_time::ptime& operator ()(
        Nexus::TelemetryService::TelemetryEvent& value) const {
      return value.m_timestamp;
    }
  };
}

#endif
