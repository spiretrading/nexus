#include <sstream>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "WebPortal/ReportJob.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("ReportJob") {
  TEST_CASE("serialization") {
    auto parameters = get<JsonObject>(parse<JsonValue>(R"({
      "range":{"start":"2026-10-01","end":"2026-10-06"},
      "entries":[{"id":1,"name":"Alice"},{"id":2,"name":"Bob"}],
      "count":5,"optional":null})"));
    auto definition = ReportDefinition("example", "Example", "Description",
      {"*"}, {{"count", "Count", "Integer", true, JsonValue(5)}},
      "report", {"--count", "{count}"},
      ReportOutputDefinition("text/csv", "csv"));
    auto job = ReportJob("job-id", DirectoryEntry::make_account(1, "Alice"),
      {DirectoryEntry::make_directory(2, "Group")}, definition, parameters,
      {"--count", "5"}, time_from_string("2026-10-01 12:00:00"),
      time_from_string("2026-10-01 12:01:00"),
      time_from_string("2026-10-01 12:02:00"), ReportJob::Status::FAILED,
      7, "Report failed.", false);
    auto expected = parse<JsonValue>(to_json(job));
    test_round_trip_shuttle(job, [&] (const auto& received) {
      REQUIRE(parse<JsonValue>(to_json(received)) == expected);
    });
    auto buffer = SharedBuffer();
    auto sender = JsonSender<SharedBuffer>();
    sender.set(Ref(buffer));
    sender.shuttle(job);
    REQUIRE(parse<JsonValue>(to_json(job)) == expected);
    auto received = ReportJob();
    received.m_parameters["stale"] = true;
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    receiver.shuttle(received);
    REQUIRE(parse<JsonValue>(to_json(received)) == expected);
    REQUIRE(!received.m_parameters.get("stale"));
  }

  TEST_CASE("status_output") {
    auto output = std::ostringstream();
    output << ReportJob::Status::QUEUED << ' ' << ReportJob::Status::RUNNING <<
      ' ' << ReportJob::Status::COMPLETED << ' ' << ReportJob::Status::FAILED <<
      ' ' << ReportJob::Status::CANCELLED << ' ' <<
      static_cast<ReportJob::Status>(99);
    REQUIRE(
      output.str() == "QUEUED RUNNING COMPLETED FAILED CANCELLED UNKNOWN(99)");
  }
}
