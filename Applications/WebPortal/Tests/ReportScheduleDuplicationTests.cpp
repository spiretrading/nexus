#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/Tests/ReportingWebServletTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  ReportSchedule make_schedule(const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Example", {}, {"*"},
      {{"count", "Count", "Integer", true}}, "report_program", {"{count}"});
    auto parameters = JsonObject();
    parameters["count"] = 5;
    return ReportSchedule("source", account, definition, parameters, {},
      time_from_string("2026-10-01 12:00:00"),
      time_from_string("2026-10-07 09:00:00"),
      time_from_string("2026-10-07 09:00:00"), {}, "America/Toronto");
  }

  HttpResponse duplicate(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    return post(servlet, session,
      "/api/reporting_service/duplicate_scheduled_report", body);
  }
}

TEST_SUITE("ReportScheduleDuplication") {
  TEST_CASE("independent_copy_and_current_definition") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto recipient = client.make_account(
      "Recipient", "", DirectoryEntry::make_directory(0));
    auto source = make_schedule(client.get_account());
    source.m_recipients = {recipient};
    auto definition = source.m_definition;
    definition.m_name = "Updated";
    definition.m_command = "updated_program";
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto local = LocalReportService(
      {definition}, client, [] (const auto&, auto) { return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    local.store(source);
    auto service = ReportService(&local);
    auto copy = duplicate_schedule(service, source.m_account, source.m_id);
    REQUIRE(copy.m_id != source.m_id);
    REQUIRE(!copy.m_id.empty());
    REQUIRE(copy.m_account == source.m_account);
    REQUIRE(copy.m_created == time.get_time());
    REQUIRE(copy.m_definition.m_name == "Updated");
    REQUIRE(copy.m_definition.m_command == "updated_program");
    REQUIRE(copy.m_parameters == source.m_parameters);
    REQUIRE(copy.m_recipients == source.m_recipients);
    REQUIRE(copy.m_start_time == source.m_start_time);
    REQUIRE(copy.m_run_time == source.m_run_time);
    REQUIRE(copy.m_time_zone == source.m_time_zone);
    REQUIRE(!copy.m_repeat_interval);
    REQUIRE(to_json(service.load_schedule(copy.m_account, copy.m_id)) ==
      to_json(copy));
    copy.m_parameters["count"] = 7;
    copy.m_recipients.clear();
    REQUIRE(service.load_schedule(copy.m_account, copy.m_id).
      m_parameters.at("count") == 5);
    service.store(copy);
    REQUIRE(to_json(service.load_schedule(source.m_account, source.m_id)) ==
      to_json(source));
    auto second = duplicate_schedule(service, source.m_account, source.m_id);
    REQUIRE(second.m_id != copy.m_id);
    REQUIRE(second.m_id != source.m_id);
    REQUIRE(local.load_schedules().size() == 3);
    REQUIRE(local.load_jobs().empty());
    REQUIRE_THROWS_AS(duplicate_schedule(service, recipient, source.m_id),
      ReportNotFoundException);
    REQUIRE(local.query(recipient, ScheduledReportQuery()).m_is_empty);
  }

  TEST_CASE("creation_rules_and_permissions") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto local = LocalReportService({source.m_definition}, client,
      [] (const auto&, auto) { return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    for(auto& start : {"2026-10-05 09:00:00", "2026-10-06 08:00:00"}) {
      source.m_start_time = time_from_string(start);
      source.m_run_time = source.m_start_time;
      local.store(source);
      REQUIRE_THROWS_AS(
        duplicate_schedule(local, source.m_account, source.m_id),
        std::invalid_argument);
      REQUIRE(local.load_schedules().size() == 1);
    }
    source.m_start_time = time_from_string("2026-01-31 09:00:00");
    source.m_run_time = time_from_string("2026-02-28 09:00:00");
    source.m_repeat_interval =
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::MONTH);
    local.store(source);
    auto copy = duplicate_schedule(local, source.m_account, source.m_id);
    REQUIRE(copy.m_start_time == source.m_start_time);
    REQUIRE(copy.m_time_zone == source.m_time_zone);
    REQUIRE(copy.m_run_time == time_from_string("2026-10-31 09:00:00"));
    REQUIRE(copy.m_repeat_interval->m_count == 1);
    REQUIRE(copy.m_repeat_interval->m_unit ==
      ReportSchedule::Interval::Unit::MONTH);
    REQUIRE(to_json(local.load_schedule(source.m_account, source.m_id)) ==
      to_json(source));
    auto definition = source.m_definition;
    definition.m_access.clear();
    local.set_definitions({definition});
    REQUIRE_THROWS_AS(duplicate_schedule(local, source.m_account, source.m_id),
      ReportNotFoundException);
    local.set_definitions({source.m_definition});
    source.m_parameters["count"] = 1.5;
    local.store(source);
    REQUIRE_THROWS_AS(duplicate_schedule(local, source.m_account, source.m_id),
      std::invalid_argument);
    auto directory = DirectoryEntry::make_directory(0);
    auto alice = client.make_account("Alice", "", directory);
    auto bob = client.make_account("Bob", "", directory);
    source.m_account = alice;
    source.m_parameters["count"] = 5;
    source.m_recipients = {bob};
    local.store(source);
    REQUIRE_THROWS_AS(duplicate_schedule(local, alice, source.m_id),
      std::invalid_argument);
    REQUIRE(local.load_schedules().size() == 2);
    REQUIRE(local.load_jobs().empty());
    local.close();
    REQUIRE_THROWS_AS(duplicate_schedule(local, alice, source.m_id),
      EndOfFileException);
  }

  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto local = LocalReportService({source.m_definition}, client,
      [] (const auto&, auto) { return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    local.store(source);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = std::string(R"({"id":"source"})");
    REQUIRE(duplicate(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    REQUIRE(local.load_schedules().size() == 1);
    auto other = sessions.create();
    other->set_account(DirectoryEntry::make_account(123));
    REQUIRE(duplicate(servlet, *other, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    session->set_account(client.get_account());
    for(auto& invalid : {"", "{", "{}", "[]", R"({"id":null})",
        R"({"id":1})", R"({"id":""})"}) {
      REQUIRE(duplicate(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(duplicate(servlet, *session, R"({"id":"missing"})").
      get_status_code() == HttpStatusCode::NOT_FOUND);
    auto response = duplicate(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_header("Content-Type").has_value());
    REQUIRE(*response.get_header("Content-Type") == "application/json");
    auto result = get<JsonObject>(parse<JsonValue>(response.get_body()));
    auto id = get<std::string>(result.at("id"));
    REQUIRE(id != source.m_id);
    REQUIRE(result.at("type") == "Example");
    REQUIRE(result.at("run_date") == "20261007");
    REQUIRE(!get<bool>(result.at("repeats")));
    auto& parameters = get<std::vector<JsonValue>>(result.at("parameters"));
    REQUIRE(parameters.size() == 1);
    auto& parameter = get<JsonObject>(parameters[0]);
    REQUIRE(parameter.at("label") == "Count");
    REQUIRE(parameter.at("value") == "5");
    for(auto& field : {"definition", "command", "arguments", "account",
        "recipients", "access", "created", "start_time", "time_zone"}) {
      REQUIRE(!result.get(field));
    }
    auto copy = local.load_schedule(source.m_account, id);
    REQUIRE(to_json(make_scheduled_report(copy)) ==
      to_string(response.get_body()));
    REQUIRE(to_json(local.load_schedule(source.m_account, source.m_id)) ==
      to_json(source));
    time.set(time_from_string("2026-10-08 12:00:00"));
    REQUIRE(duplicate(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    REQUIRE(local.load_schedules().size() == 2);
    REQUIRE(local.load_jobs().empty());
    local.set_definitions({});
    REQUIRE(duplicate(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    local.close();
    REQUIRE(duplicate(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
