#ifndef NEXUS_TELEMETRY_SERVICE_SQL_DEFINITIONS_HPP
#define NEXUS_TELEMETRY_SERVICE_SQL_DEFINITIONS_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/Row.hpp>
#include "Nexus/TelemetryService/TelemetryDataStoreException.hpp"
#include "Nexus/TelemetryService/TelemetryEvent.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {
  inline const auto& GetTelemetryEventRow() {
    static auto ROW = Viper::Row<TelemetryEvent>().
      add_column(
        "session_id", Viper::varchar(32), &TelemetryEvent::m_sessionId).
      add_column("name", Viper::varchar(256), &TelemetryEvent::m_name).
      add_column("data",
        [] (auto& row) {
          auto buffer = Beam::IO::SharedBuffer();
          auto sender =
            Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>();
          sender.SetSink(Beam::Ref(buffer));
          try {
            sender.Shuttle(row.m_data);
          } catch(const Beam::Serialization::SerializationException&) {
            BOOST_THROW_EXCEPTION(
              TelemetryDataStoreException("Unable to store data field."));
          }
          return buffer;
        },
        [] (auto& row, const auto& column) {
          if(!column.IsEmpty()) {
            auto receiver =
              Beam::Serialization::BinaryReceiver<Beam::IO::SharedBuffer>();
            receiver.SetSource(Beam::Ref(column));
            try {
              receiver.Shuttle(row.m_data);
            } catch(const Beam::Serialization::SerializationException&) {
              BOOST_THROW_EXCEPTION(
                TelemetryDataStoreException("Unable to load data field."));
            }
          }
        });
    return ROW;
  }
}

#endif
