#ifndef NEXUS_TELEMETRY_SERVICE_TELEMETRY_EVENT_HPP
#define NEXUS_TELEMETRY_SERVICE_TELEMETRY_EVENT_HPP
#include <ostream>
#include <Beam/Json/JsonObject.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Stores a single telemetry event. */
  struct TelemetryEvent {

    /** The session that produced the event. */
    std::string m_sessionId;

    /** The name of the event. */
    std::string m_name;

    /** The event's timestamp. */
    boost::posix_time::ptime m_timestamp;

    /** The data associated with the event. */
    Beam::JsonObject m_data;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const TelemetryEvent& value) {
    return out << "TelemetryEvent(" << value.m_sessionId << ", " <<
      value.m_name << ", " << value.m_timestamp << ", " << value.m_data << ')';
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::TelemetryService::TelemetryEvent> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::TelemetryService::TelemetryEvent& value, unsigned int version) {
      shuttle.Shuttle("session_id", value.m_sessionId);
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("timestamp", value.m_timestamp);
      shuttle.Shuttle("data", value.m_data);
    }
  };
}

#endif
