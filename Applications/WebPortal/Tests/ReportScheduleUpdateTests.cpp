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
    parameters["count"] = 1;
    return ReportSchedule("schedule", account, definition, parameters, {},
      time_from_string("2025-12-01 12:00:00"),
      time_from_string("2026-01-01 09:00:00"),
      time_from_string("2026-10-07 09:00:00"),
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::DAY),
      "America/Toronto");
  }

  ReportScheduleSubmission make_submission(const ReportSchedule& schedule) {
    return ReportScheduleSubmission(ReportSubmission(schedule.m_definition.m_id,
      schedule.m_parameters, schedule.m_recipients),
      schedule.m_start_time, schedule.m_repeat_interval, schedule.m_time_zone);
  }

  HttpResponse update(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    return post(
      servlet, session, "/api/reporting_service/update_scheduled_report", body);
  }
}

TEST_SUITE("ReportScheduleUpdate") {
  TEST_CASE("changed_date_rule") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto schedule = make_schedule(client.get_account());
    schedule.m_repeat_interval = ReportSchedule::Interval(1,
      ReportSchedule::Interval::Unit::MONTH,
      MonthBoundaryDateRule(0, MonthBoundaryDateRule::Boundary::FIRST, 0));
    auto submission = make_submission(schedule);
    submission.m_repeat_interval->m_rule =
      MonthBoundaryDateRule(0, MonthBoundaryDateRule::Boundary::LAST, -1);
    auto updated = prepare_report_schedule(schedule, submission,
      {schedule.m_definition}, client,
      time_from_string("2026-10-08 12:00:00"));
    REQUIRE(updated.m_run_time == time_from_string("2026-10-30 09:00:00"));
    auto encoded = parse<JsonValue>(to_json(*updated.m_repeat_interval));
    auto decoded = parse_report_interval(encoded);
    REQUIRE(to_json(decoded) == to_json(*updated.m_repeat_interval));
  }

  TEST_CASE("settings_and_unchanged_timing") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto recipient =
      client.make_account("Recipient", "", DirectoryEntry::make_directory(0));
    auto schedule = make_schedule(client.get_account());
    schedule.m_recipients = {client.get_account()};
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto executions = 0;
    auto local = LocalReportService({schedule.m_definition}, client,
      [&] (const auto&, auto) { ++executions; return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    local.store(schedule);
    auto service = ReportService(&local);
    auto submission = make_submission(schedule);
    submission.m_report.m_parameters["count"] = 5;
    auto forged = recipient;
    forged.m_name = "Forged";
    submission.m_report.m_recipients = {forged, recipient};
    submission.m_time_zone = "America/Vancouver";
    submission.m_start_time = time_from_string("2026-01-01 06:00:00");
    service.update_schedule(schedule.m_account, schedule.m_id, submission);
    auto saved = service.load_schedule(schedule.m_account, schedule.m_id);
    REQUIRE(saved.m_parameters.at("count") == 5);
    REQUIRE(saved.m_recipients == std::vector({recipient}));
    REQUIRE(saved.m_recipients[0].m_name == "Recipient");
    REQUIRE(saved.m_id == schedule.m_id);
    REQUIRE(saved.m_account == schedule.m_account);
    REQUIRE(saved.m_created == schedule.m_created);
    REQUIRE(saved.m_start_time == schedule.m_start_time);
    REQUIRE(saved.m_run_time == schedule.m_run_time);
    REQUIRE(saved.m_time_zone == schedule.m_time_zone);
    REQUIRE(executions == 0);
    REQUIRE(local.load_jobs().empty());
    schedule.m_repeat_interval.reset();
    schedule.m_run_time = schedule.m_start_time;
    local.store(schedule);
    submission = make_submission(schedule);
    submission.m_report.m_parameters["count"] = 2;
    service.update_schedule(schedule.m_account, schedule.m_id, submission);
    saved = service.load_schedule(schedule.m_account, schedule.m_id);
    REQUIRE(saved.m_run_time == schedule.m_start_time);
    REQUIRE(saved.m_parameters.at("count") == 2);
    schedule.m_time_zone = "UTC";
    schedule.m_start_time = time_from_string("2026-11-01 06:30:00");
    schedule.m_run_time = schedule.m_start_time;
    time.set(time_from_string("2026-11-02 12:00:00"));
    local.store(schedule);
    submission = make_submission(schedule);
    submission.m_time_zone = "America/Toronto";
    submission.m_start_time = time_from_string("2026-11-01 01:30:00");
    submission.m_report.m_parameters["count"] = 4;
    service.update_schedule(schedule.m_account, schedule.m_id, submission);
    saved = service.load_schedule(schedule.m_account, schedule.m_id);
    REQUIRE(saved.m_start_time == schedule.m_start_time);
    REQUIRE(saved.m_run_time == schedule.m_run_time);
    REQUIRE(saved.m_time_zone == "UTC");
    REQUIRE(saved.m_parameters.at("count") == 4);
  }

  TEST_CASE("changed_timing_and_validation") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto schedule = make_schedule(client.get_account());
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto local = LocalReportService({schedule.m_definition}, client,
      [] (const auto&, auto) { return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    local.store(schedule);
    auto submission = make_submission(schedule);
    submission.m_start_time = time_from_string("2026-01-31 10:00:00");
    submission.m_repeat_interval =
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::MONTH);
    local.update_schedule(schedule.m_account, schedule.m_id, submission);
    auto saved = local.load_schedule(schedule.m_account, schedule.m_id);
    REQUIRE(saved.m_run_time == time_from_string("2026-10-31 10:00:00"));
    auto original = to_json(saved);
    submission.m_repeat_interval.reset();
    REQUIRE_THROWS_AS(local.update_schedule(
      schedule.m_account, schedule.m_id, submission), std::invalid_argument);
    REQUIRE(to_json(local.load_schedule(schedule.m_account, schedule.m_id)) ==
      original);
    submission.m_start_time = time_from_string("2026-10-07 10:00:00");
    local.update_schedule(schedule.m_account, schedule.m_id, submission);
    saved = local.load_schedule(schedule.m_account, schedule.m_id);
    REQUIRE(!saved.m_repeat_interval);
    REQUIRE(saved.m_run_time == submission.m_start_time);
    original = to_json(saved);
    submission.m_report.m_parameters["count"] = 1.5;
    REQUIRE_THROWS_AS(local.update_schedule(
      schedule.m_account, schedule.m_id, submission), std::invalid_argument);
    submission.m_report.m_parameters["count"] = 1;
    submission.m_report.m_recipients = {DirectoryEntry::STAR_DIRECTORY};
    REQUIRE_THROWS_AS(local.update_schedule(
      schedule.m_account, schedule.m_id, submission), std::invalid_argument);
    REQUIRE(to_json(local.load_schedule(schedule.m_account, schedule.m_id)) ==
      original);
    submission.m_report.m_recipients.clear();
    local.set_definitions({});
    REQUIRE_THROWS_AS(local.update_schedule(
      schedule.m_account, schedule.m_id, submission), ReportNotFoundException);
    REQUIRE(to_json(local.load_schedule(schedule.m_account, schedule.m_id)) ==
      original);
    REQUIRE_THROWS_AS(local.update_schedule(
      DirectoryEntry::make_account(123), schedule.m_id, submission),
      ReportNotFoundException);
  }

  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto schedule = make_schedule(client.get_account());
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto local = LocalReportService({schedule.m_definition}, client,
      [] (const auto&, auto) { return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    local.store(schedule);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto text = std::string(R"({"id":"schedule","report_type":"example",
      "parameters":{"count":3},"recipients":[],"scheduled":true,
      "schedule_date_time":"20261007T090000","repeats":false,
      "repeat_interval":null,"time_zone":"America/Toronto"})");
    REQUIRE(update(servlet, *session, text).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    auto response = update(servlet, *session, text);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    auto saved = local.load_schedule(schedule.m_account, schedule.m_id);
    REQUIRE(saved.m_parameters.at("count") == 3);
    REQUIRE(saved.m_time_zone == "America/Toronto");
    REQUIRE(saved.m_start_time == time_from_string("2026-10-07 09:00:00"));
    REQUIRE(!saved.m_repeat_interval);
    for(auto& invalid : {"", "{", "{}"}) {
      REQUIRE(update(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    auto inputs = std::vector({
      std::pair("time_zone", JsonValue("Not/AZone")),
      std::pair("scheduled", JsonValue(false)),
      std::pair("schedule_date_time", JsonValue("20261005T090000")),
      std::pair("schedule_date_time", JsonValue("20260308T023000")),
      std::pair("repeats", JsonValue(true)),
      std::pair("parameters", JsonValue(JsonObject()))});
    for(auto& [name, value] : inputs) {
      auto body = get<JsonObject>(parse<JsonValue>(text));
      body[name] = value;
      REQUIRE(update(servlet, *session, to_string(JsonValue(body))).
        get_status_code() == HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(to_json(local.load_schedule(schedule.m_account, schedule.m_id)) ==
      to_json(saved));
    auto body = get<JsonObject>(parse<JsonValue>(text));
    body["id"] = JsonValue("missing");
    REQUIRE(update(servlet, *session, to_string(JsonValue(body))).
      get_status_code() == HttpStatusCode::NOT_FOUND);
    local.close();
    REQUIRE(update(servlet, *session, text).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
