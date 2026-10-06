#include <limits>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportSchedule make_schedule(std::string id, const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Example", {}, {"*"},
      {{"currency", "Currency", "Currency", true},
        {"account", "Account", "DirectoryEntry", true},
        {"omitted", "Omitted", "String", false}},
      "private_command", {"private_argument"},
      ReportOutputDefinition("text/csv", "csv"));
    auto parameters = JsonObject();
    parameters["currency"] = 840;
    parameters["account"] = parse<JsonValue>(to_json(account));
    return ReportSchedule(id, account, definition, parameters, {},
      time_from_string("2026-10-06 10:00:00"),
      time_from_string("2026-10-07 12:00:00"),
      time_from_string("2026-10-07 12:00:00"));
  }

  HttpResponse query(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(
      HttpMethod::POST, Uri("/api/reporting_service/query_scheduled_reports"),
      from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing scheduled report query route.");
  }
}

TEST_SUITE("ScheduledReport") {
  TEST_CASE("ownership_and_empty_state") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto service = ReportService(&local);
    auto account = client.get_account();
    REQUIRE(service.query(account, ScheduledReportQuery()).m_is_empty);
    auto other = make_schedule("other", DirectoryEntry::make_account(123));
    other.m_recipients = {account};
    service.store(other);
    REQUIRE(service.query(account, ScheduledReportQuery()).m_is_empty);
    auto mine = make_schedule("mine", account);
    service.store(mine);
    auto page = service.query(account, ScheduledReportQuery());
    REQUIRE(!page.m_is_empty);
    REQUIRE(page.m_filtered_count == 1);
    REQUIRE(page.m_schedules.size() == 1);
    REQUIRE(page.m_schedules[0].m_id == mine.m_id);
    page = service.query(account, ScheduledReportQuery("unmatched", 0));
    REQUIRE(!page.m_is_empty);
    REQUIRE(page.m_filtered_count == 0);
    REQUIRE(page.m_schedules.empty());
    REQUIRE_THROWS_AS(
      service.query(DirectoryEntry::make_directory(0), ScheduledReportQuery()),
      std::invalid_argument);
    local.close();
    REQUIRE_THROWS_AS(
      service.query(account, ScheduledReportQuery()), EndOfFileException);
  }

  TEST_CASE("filtering_pagination_and_order") {
    auto account = DirectoryEntry::make_account(1, "Alice");
    auto schedules = std::vector<ReportSchedule>();
    for(auto i = 0; i != 101; ++i) {
      auto schedule = make_schedule(std::to_string(i), account);
      schedule.m_created += seconds(i);
      if(i % 2 == 0) {
        schedule.m_repeat_interval =
          ReportSchedule::Interval(2, ReportSchedule::Interval::Unit::MONTH);
        schedule.m_run_time = time_from_string("2026-12-07 12:00:00");
      }
      schedules.push_back(schedule);
    }
    auto page =
      query_scheduled_reports(schedules, account, ScheduledReportQuery());
    REQUIRE(page.m_filtered_count == 101);
    REQUIRE(page.m_schedules.size() == 50);
    REQUIRE(page.m_schedules.front().m_id == "100");
    REQUIRE(page.m_schedules.back().m_id == "51");
    page =
      query_scheduled_reports(schedules, account, ScheduledReportQuery({}, 2));
    REQUIRE(page.m_schedules.size() == 1);
    REQUIRE(page.m_schedules[0].m_id == "0");
    page = query_scheduled_reports(schedules, account,
      ScheduledReportQuery({}, std::numeric_limits<std::uint32_t>::max()));
    REQUIRE(page.m_schedules.empty());
    REQUIRE(page.m_filtered_count == 101);
    for(auto& text : {" example ", "usd", "CURRENCY", "alice", "Account"}) {
      page = query_scheduled_reports(
        schedules, account, ScheduledReportQuery(text, 0));
      REQUIRE(page.m_filtered_count == 101);
    }
    for(auto& text : {"recurring", "Dec 07, 2026"}) {
      page = query_scheduled_reports(
        schedules, account, ScheduledReportQuery(text, 0));
      REQUIRE(page.m_filtered_count == 51);
      REQUIRE(page.m_schedules.front().m_is_repeating);
      REQUIRE(page.m_schedules.front().m_run_date == date(2026, 12, 7));
    }
    page = query_scheduled_reports(
      schedules, account, ScheduledReportQuery("Oct 07, 2026", 0));
    REQUIRE(page.m_filtered_count == 50);
    REQUIRE(!page.m_schedules.front().m_is_repeating);
    auto first = make_schedule("a", account);
    auto second = make_schedule("b", account);
    page = query_scheduled_reports({second, first}, account, {});
    REQUIRE(page.m_schedules[0].m_id == "a");
  }

  TEST_CASE("snapshot_and_serialization") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto schedule = make_schedule("schedule", client.get_account());
    schedule.m_recipients = {client.get_account()};
    schedule.m_repeat_interval =
      ReportSchedule::Interval(3, ReportSchedule::Interval::Unit::WEEK);
    local.store(schedule);
    test_round_trip_shuttle(schedule, [&] (const auto& received) {
      REQUIRE(to_json(received) == to_json(schedule));
    });
    schedule.m_parameters["currency"] = 124;
    REQUIRE(local.load_schedules()[0].m_parameters.at("currency") == 840);
    auto snapshot = local.load_schedules();
    snapshot[0].m_parameters["currency"] = 826;
    REQUIRE(local.load_schedules()[0].m_parameters.at("currency") == 840);
    schedule.m_definition.m_name = "Updated";
    local.store(schedule);
    REQUIRE(local.load_schedules().size() == 1);
    auto page = local.query(client.get_account(), ScheduledReportQuery());
    REQUIRE(page.m_schedules[0].m_type == "Updated");
    REQUIRE(page.m_schedules[0].m_parameters[0].m_value == "CAD");
    test_round_trip_shuttle(page.m_schedules[0], [&] (const auto& received) {
      REQUIRE(to_json(received) == to_json(page.m_schedules[0]));
    });
    local.close();
    REQUIRE_THROWS_AS(local.store(schedule), EndOfFileException);
  }

  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    local.store(make_schedule("schedule", client.get_account()));
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = std::string(R"({"filters":{"query":""},"page_index":0})");
    REQUIRE(query(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    for(auto& invalid : {"", "{", "{}", R"({"filters":{},"page_index":0})",
        R"({"filters":{"query":1},"page_index":0})",
        R"({"filters":{"query":""},"page_index":null})",
        R"({"filters":{"query":""},"page_index":-1})",
        R"({"filters":{"query":""},"page_index":0.5})",
        R"({"filters":{"query":""},"page_index":4294967296})"}) {
      REQUIRE(query(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    auto response = query(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto text = to_string(response.get_body());
    REQUIRE(text.find("private") == std::string::npos);
    auto result = get<JsonObject>(parse<JsonValue>(text));
    REQUIRE(result.at("status") == 1);
    REQUIRE(!get<bool>(result.at("is_empty")));
    REQUIRE(result.at("filtered_count") == 1);
    auto& list = get<std::vector<JsonValue>>(result.at("schedules"));
    REQUIRE(list.size() == 1);
    auto& entry = get<JsonObject>(list[0]);
    REQUIRE(entry.at("id") == "schedule");
    REQUIRE(entry.at("type") == "Example");
    REQUIRE(!get<bool>(entry.at("repeats")));
    REQUIRE(entry.at("run_date") == "20261007");
    auto& parameters = get<std::vector<JsonValue>>(entry.at("parameters"));
    REQUIRE(parameters.size() == 2);
    REQUIRE(get<JsonObject>(parameters[0]).at("label") == "Currency");
    REQUIRE(get<JsonObject>(parameters[0]).at("value") == "USD");
    REQUIRE(!entry.get("definition"));
    REQUIRE(!entry.get("account"));
    REQUIRE(!entry.get("recipients"));
    local.close();
    REQUIRE(query(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
