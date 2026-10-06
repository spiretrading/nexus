#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"
#include "WebPortal/ReportSubmission.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportDefinition make_definition() {
    return parse_report_definition(YAML::Load(R"(
id: example
name: Example
access: ['*']
parameters:
  - {name: account, label: Account, type: DirectoryEntry, required: true}
  - {name: currency, label: Currency, type: Currency, default: USD}
  - {name: count, label: Count, type: Integer, default: 0}
  - {name: period, label: Period, type: DateRange}
command: report_program
arguments:
  - '--account={account.id}'
  - '{currency.code}'
  - if_present: count
    arguments: ['--count', '{count}']
  - if_present: period.start
    arguments: ['--start', '{period.start}']
output: {media_type: text/csv, extension: csv}
)"));
  }

  JsonObject make_parameters(const DirectoryEntry& account) {
    auto result = JsonObject();
    result["account"] = parse<JsonValue>(to_json(account));
    return result;
  }

  HttpResponse submit(ReportingWebServlet& servlet,
      const WebPortalSession& session, const JsonObject& body) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/submit_report"),
      from<SharedBuffer>(to_string(body)));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report submission route.");
  }
}

TEST_SUITE("ReportSubmission") {
  TEST_CASE("schedule_inputs") {
    auto expected = time_from_string("2026-10-07 12:30:45.123456");
    REQUIRE(parse_report_datetime("20261007T123045.123456") == expected);
    REQUIRE(parse_report_datetime("2026-10-07T12:30:45.123456") == expected);
    for(auto& invalid : {"", "+infinity", "not-a-date-time",
        "20261007T240000", "20261007T126000", "20261007T123060",
        "20261007T123000Z", "20261007T123000junk"}) {
      REQUIRE_THROWS_AS(parse_report_datetime(invalid), std::invalid_argument);
    }
    auto interval =
      parse_report_interval(parse<JsonValue>(R"({"count":2,"unit":2})"));
    REQUIRE(interval.m_count == 2);
    REQUIRE(interval.m_unit == ReportSchedule::Interval::Unit::MONTH);
    for(auto& invalid : {R"({"count":0,"unit":0})",
        R"({"count":1.5,"unit":0})", R"({"count":1,"unit":4})",
        R"({"count":1,"unit":0.5})", R"({"count":4294967296,"unit":0})",
        R"({"count":null,"unit":0})", "{}"}) {
      REQUIRE_THROWS_AS(parse_report_interval(parse<JsonValue>(invalid)),
        std::invalid_argument);
    }
  }

  TEST_CASE("defaults_and_arguments") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto definition = make_definition();
    auto values = prepare_report_parameters(definition,
      make_parameters(client.get_account()), client.get_account(), client);
    REQUIRE(values.at("currency") == 840);
    REQUIRE(values.at("count") == 0);
    REQUIRE(!values.get("period"));
    REQUIRE(make_report_arguments(definition, values) ==
      std::vector<std::string>({"--account=1", "840", "--count", "0"}));
    auto input = make_parameters(client.get_account());
    input["currency"] = 124;
    input["period"] = parse<JsonValue>(
      R"({"start":"2026-10-01","end":"20261005"})");
    values = prepare_report_parameters(
      definition, input, client.get_account(), client);
    REQUIRE(make_report_arguments(definition, values) ==
      std::vector<std::string>(
        {"--account=1", "124", "--count", "0", "--start", "20261001"}));
    input["currency"] = JsonNull();
    REQUIRE(prepare_report_parameters(definition, input,
      client.get_account(), client).at("currency") == 840);
    definition.m_arguments = {"literal with spaces; $(echo hi)", ""};
    REQUIRE(make_report_arguments(definition, values) ==
      std::vector<std::string>({"literal with spaces; $(echo hi)", ""}));
    definition.m_arguments = {"{period.end}", "{period.missing}"};
    REQUIRE_THROWS_AS(make_report_arguments(definition, values),
      std::invalid_argument);
    definition.m_arguments = {"{unknown}"};
    REQUIRE_THROWS_AS(make_report_arguments(definition, values),
      std::runtime_error);
  }

  TEST_CASE("parameter_validation") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto definition = make_definition();
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, JsonObject(),
      client.get_account(), client), std::invalid_argument);
    auto input = make_parameters(client.get_account());
    input["count"] = 1.5;
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, input,
      client.get_account(), client), std::invalid_argument);
    input["count"] = false;
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, input,
      client.get_account(), client), std::invalid_argument);
    input["count"] = -4;
    input.set("currency", "BAD");
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, input,
      client.get_account(), client), std::invalid_argument);
    input.set("currency", "CAD");
    input["period"] = parse<JsonValue>(R"({"start":"20260230"})");
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, input,
      client.get_account(), client), std::invalid_argument);
    input["period"] = parse<JsonValue>(
      R"({"start":"20261005","end":"20261001"})");
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, input,
      client.get_account(), client), std::invalid_argument);
  }

  TEST_CASE("shared_parameter_types") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto definition = make_definition();
    definition.m_parameters = {
      {"date", "Date", "Date", true, {}},
      {"time", "Time", "Time", true, {}},
      {"timestamp", "Timestamp", "DateTime", true, {}},
      {"money", "Money", "Money", true, {}},
      {"scope", "Scope", "Scope", true, {}},
      {"decimal", "Decimal", "Decimal", true, {}}};
    auto input = get<JsonObject>(parse<JsonValue>(R"({
      "date":"2026-10-05", "time":"12:34:56.125",
      "timestamp":"20261005T123456", "money":"1.25",
      "scope":"*", "decimal":0.000000125})"));
    auto values = prepare_report_parameters(
      definition, input, client.get_account(), client);
    REQUIRE(values.at("date") == "20261005");
    REQUIRE(values.at("timestamp") == "20261005T123456");
    REQUIRE(get<bool>(get<JsonObject>(values.at("scope")).at("is_global")));
    REQUIRE(values.at("decimal") == 0.000000125);
    input["scope"] = parse<JsonValue>(R"({"name":"", "is_global":false,
      "countries":[], "venues":[],
      "tickers":[{"symbol":"UNLISTED","venue":"XNAS"}]})");
    REQUIRE_NOTHROW(prepare_report_parameters(
      definition, input, client.get_account(), client));
    auto& tickers = get<std::vector<JsonValue>>(
      get<JsonObject>(input["scope"])["tickers"]);
    get<JsonObject>(tickers[0]).set("symbol", "bad symbol");
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, input,
      client.get_account(), client), std::invalid_argument);
    input.set("scope", "*");
    input.set("time", "12:99:00");
    REQUIRE_THROWS_AS(prepare_report_parameters(definition, input,
      client.get_account(), client), std::invalid_argument);
  }

  TEST_CASE("directory_permissions") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto directory = DirectoryEntry::make_directory(0);
    auto alice = client.make_account("alice", "", directory);
    auto bob = client.make_account("bob", "", directory);
    auto input = parse<JsonValue>(to_json(bob));
    REQUIRE_THROWS_AS(resolve_report_entry(input, alice, client, true),
      std::invalid_argument);
    client.store(alice, bob, Permission::READ);
    get<JsonObject>(input).set("name", "forged");
    REQUIRE(resolve_report_entry(input, alice, client, true).m_name == "bob");
    get<JsonObject>(input)["type"] = 1;
    REQUIRE_THROWS_AS(resolve_report_entry(input, alice, client, true),
      std::invalid_argument);
    input = parse<JsonValue>(to_json(DirectoryEntry::STAR_DIRECTORY));
    REQUIRE(resolve_report_entry(input, alice, client, true) ==
      DirectoryEntry::STAR_DIRECTORY);
    REQUIRE_THROWS_AS(resolve_report_entry(input, alice, client, false),
      std::invalid_argument);
  }

  TEST_CASE("submission_endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto sessions = WebSessionStore<WebPortalSession>();
    auto definition = make_definition();
    auto reports = LocalReportService({definition}, client,
      [] (const auto&, auto) { return 0; },
      TimeClient(std::in_place_type<LocalTimeClient>), 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    auto servlet = ReportingWebServlet(Ref(sessions), &reports);
    auto session = sessions.create();
    auto body = JsonObject();
    body.set("report_type", "example");
    body["parameters"] = make_parameters(client.get_account());
    body["recipients"] = std::vector<JsonValue>();
    body["scheduled"] = false;
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    REQUIRE(reports.load_jobs().empty());
    session->set_account(client.get_account());
    auto response = submit(servlet, *session, body);
    INFO(to_string(response.get_body()));
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto id = get<std::string>(parse<JsonValue>(response.get_body()));
    auto jobs = reports.load_jobs();
    REQUIRE(jobs.size() == 1);
    REQUIRE(jobs.back().m_id == id);
    REQUIRE(jobs.back().m_account == client.get_account());
    REQUIRE(jobs.back().m_parameters.at("currency") == 840);
    definition.m_command = "updated_program";
    reports.set_definitions({definition});
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::OK);
    REQUIRE(reports.load_jobs().back().m_definition.m_command ==
      "updated_program");
    body["report_type"] = 5;
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body.set("report_type", "example");
    body["scheduled"] = JsonNull();
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body["scheduled"] = true;
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body["scheduled"] = false;
    body["parameters"] = JsonObject();
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body["parameters"] = make_parameters(client.get_account());
    body["recipients"] = JsonNull();
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body["recipients"] = std::vector<JsonValue>({parse<JsonValue>(
      R"({"type":0,"id":1.5,"name":"invalid"})")});
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body["recipients"] = std::vector<JsonValue>({JsonObject()});
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body["recipients"] = std::vector<JsonValue>({parse<JsonValue>(
      R"({"type":0,"id":1234567,"name":"missing"})")});
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    body["recipients"] = std::vector<JsonValue>();
    definition.m_access = {};
    reports.set_definitions({definition});
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    REQUIRE(reports.load_jobs().size() == 2);
  }

  TEST_CASE("scheduled_submission") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto recipient =
      client.make_account("Recipient", "", DirectoryEntry::make_directory(0));
    auto definition = make_definition();
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto executions = std::atomic_int(0);
    auto local = LocalReportService({definition}, client,
      [&] (const auto&, auto) { ++executions; return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    auto service = ReportService(&local);
    auto submission = ReportScheduleSubmission(
      ReportSubmission("example", make_parameters(client.get_account()),
        {recipient, recipient}), time_from_string("2026-10-06 09:00:00"),
      {}, "America/Toronto");
    auto id = service.submit(client.get_account(), submission);
    auto schedule = service.load_schedule(client.get_account(), id);
    REQUIRE(!id.empty());
    REQUIRE(schedule.m_account == client.get_account());
    REQUIRE(schedule.m_created == time.get_time());
    REQUIRE(schedule.m_parameters.at("currency") == 840);
    REQUIRE(schedule.m_parameters.at("count") == 0);
    REQUIRE(schedule.m_recipients == std::vector({recipient}));
    REQUIRE(schedule.m_start_time == submission.m_start_time);
    REQUIRE(schedule.m_run_time == submission.m_start_time);
    REQUIRE(schedule.m_time_zone == "America/Toronto");
    REQUIRE(!schedule.m_repeat_interval);
    REQUIRE(service.query(
      client.get_account(), ScheduledReportQuery()).m_filtered_count == 1);
    submission.m_start_time = time_from_string("2026-01-31 09:00:00");
    submission.m_repeat_interval =
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::MONTH);
    definition.m_command = "updated_program";
    local.set_definitions({definition});
    auto recurring = service.submit(client.get_account(), submission);
    REQUIRE(recurring != id);
    schedule = service.load_schedule(client.get_account(), recurring);
    REQUIRE(schedule.m_start_time == submission.m_start_time);
    REQUIRE(schedule.m_run_time == time_from_string("2026-10-31 09:00:00"));
    REQUIRE(schedule.m_definition.m_command == "updated_program");
    REQUIRE(schedule.m_repeat_interval->m_count == 1);
    REQUIRE(schedule.m_repeat_interval->m_unit ==
      ReportSchedule::Interval::Unit::MONTH);
    submission.m_repeat_interval.reset();
    for(auto& start : {"2026-10-05 09:00:00", "2026-10-06 08:00:00"}) {
      submission.m_start_time = time_from_string(start);
      REQUIRE_THROWS_AS(service.submit(client.get_account(), submission),
        std::invalid_argument);
    }
    submission.m_start_time = time_from_string("2026-10-07 09:00:00");
    submission.m_repeat_interval =
      ReportSchedule::Interval(0, ReportSchedule::Interval::Unit::DAY);
    REQUIRE_THROWS_AS(
      service.submit(client.get_account(), submission), std::invalid_argument);
    submission.m_repeat_interval.reset();
    submission.m_report.m_recipients = {DirectoryEntry::STAR_DIRECTORY};
    REQUIRE_THROWS_AS(
      service.submit(client.get_account(), submission), std::invalid_argument);
    submission.m_report.m_recipients.clear();
    submission.m_report.m_parameters["count"] = 1.5;
    REQUIRE_THROWS_AS(
      service.submit(client.get_account(), submission), std::invalid_argument);
    submission.m_report.m_parameters["count"] = 1;
    definition.m_access.clear();
    local.set_definitions({definition});
    REQUIRE_THROWS_AS(service.submit(client.get_account(), submission),
      ReportNotFoundException);
    REQUIRE(local.load_schedules().size() == 2);
    REQUIRE(local.load_jobs().empty());
    REQUIRE(executions.load() == 0);
    local.close();
    REQUIRE_THROWS_AS(service.submit(client.get_account(), submission),
      EndOfFileException);
  }

  TEST_CASE("scheduled_submission_endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto reports = LocalReportService({make_definition()}, client,
      [] (const auto&, auto) { return 0; }, &time, 1,
      Timer(std::in_place_type<TriggerTimer>),
      Timer(std::in_place_type<TriggerTimer>));
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &reports);
    auto body = get<JsonObject>(parse<JsonValue>(R"({
      "report_type":"example", "parameters":{}, "recipients":[],
      "scheduled":true, "schedule_date_time":"20261007T090000",
      "repeats":false, "repeat_interval":null,
      "time_zone":"America/Toronto"})"));
    body["parameters"] = make_parameters(client.get_account());
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    auto response = submit(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto id = get<std::string>(parse<JsonValue>(response.get_body()));
    auto schedule = reports.load_schedule(client.get_account(), id);
    REQUIRE(!schedule.m_repeat_interval);
    REQUIRE(schedule.m_start_time == time_from_string("2026-10-07 09:00:00"));
    REQUIRE(schedule.m_time_zone == "America/Toronto");
    auto valid = to_string(body);
    auto inputs = std::vector({
      std::pair("time_zone", JsonValue("Not/AZone")),
      std::pair("time_zone", JsonValue("")),
      std::pair("time_zone", JsonValue(JsonNull())),
      std::pair("schedule_date_time", JsonValue("20261005T090000")),
      std::pair("schedule_date_time", JsonValue("20270314T023000")),
      std::pair("schedule_date_time", JsonValue(JsonNull())),
      std::pair("repeats", JsonValue(true)),
      std::pair(
        "repeat_interval", parse<JsonValue>(R"({"count":1,"unit":0})"))});
    for(auto& [name, value] : inputs) {
      body = get<JsonObject>(parse<JsonValue>(valid));
      body[name] = value;
      REQUIRE(submit(servlet, *session, body).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(reports.load_schedules().size() == 1);
    body = get<JsonObject>(parse<JsonValue>(valid));
    body["repeats"] = true;
    body.set("schedule_date_time", "20260131T090000");
    body["repeat_interval"] = parse<JsonValue>(R"({"count":1,"unit":2})");
    response = submit(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto recurring = get<std::string>(parse<JsonValue>(response.get_body()));
    REQUIRE(recurring != id);
    schedule = reports.load_schedule(client.get_account(), recurring);
    REQUIRE(schedule.m_run_time == time_from_string("2026-10-31 09:00:00"));
    REQUIRE(reports.load_jobs().empty());
    reports.set_definitions({});
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    REQUIRE(reports.load_schedules().size() == 2);
    reports.close();
    REQUIRE(submit(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

}
