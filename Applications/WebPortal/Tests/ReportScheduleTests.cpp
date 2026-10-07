#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
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
      {{"account", "Account", "DirectoryEntry", true},
        {"count", "Count", "Integer", false}}, "private_command",
      {"private_argument"}, ReportOutputDefinition("text/csv", "csv"));
    auto parameters = JsonObject();
    parameters["account"] =
      parse<JsonValue>(to_json(DirectoryEntry::STAR_DIRECTORY));
    parameters["count"] = 0;
    return ReportSchedule("schedule", account, definition, parameters, {},
      time_from_string("2026-10-06 10:00:00"),
      time_from_string("2026-10-07 12:30:00"),
      time_from_string("2026-12-07 12:30:00"),
      ReportSchedule::Interval(2, ReportSchedule::Interval::Unit::MONTH));
  }

  HttpResponse load(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    return post(
      servlet, session, "/api/reporting_service/load_scheduled_report", body);
  }
}

TEST_SUITE("ReportSchedule") {
  TEST_CASE("owner_only_snapshots") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto group =
      client.make_directory("Group", DirectoryEntry::make_directory(0));
    auto member = client.make_account("Member", "", group);
    auto direct =
      client.make_account("Direct", "", DirectoryEntry::make_directory(0));
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto service = ReportService(&local);
    auto schedule = make_schedule(client.get_account());
    schedule.m_recipients = {group, direct};
    service.store(schedule);
    REQUIRE_THROWS_AS(service.load_schedule(schedule.m_account, "missing"),
      ReportNotFoundException);
    REQUIRE_THROWS_AS(service.load_schedule(DirectoryEntry(), schedule.m_id),
      ReportNotFoundException);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    for(auto& account : {group, member, direct}) {
      REQUIRE_THROWS_AS(
        service.load_schedule(account, schedule.m_id), ReportNotFoundException);
      auto session = sessions.create();
      session->set_account(account);
      auto response =
        load(servlet, *session,R"({"id":"schedule","time_zone":"UTC"})");
      REQUIRE(response.get_status_code() == HttpStatusCode::NOT_FOUND);
    }
    auto snapshot = service.load_schedule(schedule.m_account, schedule.m_id);
    REQUIRE(to_json(snapshot) == to_json(schedule));
    snapshot.m_parameters["count"] = 5;
    snapshot.m_recipients.clear();
    snapshot.m_repeat_interval->m_count = 3;
    REQUIRE(to_json(service.load_schedule(schedule.m_account, schedule.m_id)) ==
      to_json(schedule));
    local.set_definitions({});
    REQUIRE(to_json(service.load_schedule(schedule.m_account, schedule.m_id)) ==
      to_json(schedule));
    local.close();
    REQUIRE_THROWS_AS(service.load_schedule(schedule.m_account, schedule.m_id),
      EndOfFileException);
  }

  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto schedule = make_schedule(client.get_account());
    schedule.m_recipients = {client.get_account()};
    local.store(schedule);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = std::string(R"({"id":"schedule","time_zone":"UTC"})");
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    for(auto& invalid : {"", "{", "{}", "[]", R"({"id":null})",
        R"({"id":1})", R"({"id":"","time_zone":"UTC"})",
        R"({"id":"schedule"})", R"({"id":"schedule","time_zone":null})",
        R"({"id":"schedule","time_zone":""})"}) {
      REQUIRE(load(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(load(servlet, *session,
      R"({"id":"missing","time_zone":"UTC"})").get_status_code() ==
        HttpStatusCode::NOT_FOUND);
    auto response = load(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_header("Content-Type").has_value());
    REQUIRE(*response.get_header("Content-Type") == "application/json");
    REQUIRE(response.get_header("Cache-Control").has_value());
    REQUIRE(*response.get_header("Cache-Control") == "private, no-store");
    auto text = to_string(response.get_body());
    REQUIRE(text.find("private") == std::string::npos);
    auto result = get<JsonObject>(parse<JsonValue>(text));
    REQUIRE(result.at("report_type") == schedule.m_definition.m_id);
    REQUIRE(get<bool>(result.at("scheduled")));
    REQUIRE(get<bool>(result.at("repeats")));
    REQUIRE(result.at("schedule_date_time") == "20261007T123000");
    auto& interval = get<JsonObject>(result.at("repeat_interval"));
    REQUIRE(interval.at("count") == 2);
    REQUIRE(interval.at("unit") == 2);
    auto& parameters = get<JsonObject>(result.at("parameters"));
    REQUIRE(parameters.at("count") == 0);
    REQUIRE(parameters.at("account") ==
      parse<JsonValue>(to_json(DirectoryEntry::STAR_DIRECTORY)));
    auto& recipients = get<std::vector<JsonValue>>(result.at("recipients"));
    REQUIRE(recipients.size() == 1);
    REQUIRE(recipients[0] == parse<JsonValue>(to_json(client.get_account())));
    for(auto& field : {"definition", "account", "command", "arguments",
        "access", "created", "run_time"}) {
      REQUIRE(!result.get(field));
    }
    response = load(
      servlet, *session, R"({"id":"schedule","time_zone":"America/Toronto"})");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    result = get<JsonObject>(parse<JsonValue>(response.get_body()));
    REQUIRE(result.at("schedule_date_time") == "20261007T083000");
    REQUIRE(load(servlet, *session,
      R"({"id":"schedule","time_zone":"Not/AZone"})").get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    schedule.m_repeat_interval.reset();
    local.store(schedule);
    response = load(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    result = get<JsonObject>(parse<JsonValue>(response.get_body()));
    REQUIRE(!get<bool>(result.at("repeats")));
    REQUIRE(std::get_if<JsonNull>(&result.at("repeat_interval")));
    local.close();
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
