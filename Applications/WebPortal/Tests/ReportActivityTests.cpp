#include <limits>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Money.hpp"
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportJob make_job(std::string id, const DirectoryEntry& account) {
    auto definition =
      ReportDefinition("example", "Example", {}, {}, {}, "private_command");
    return ReportJob(id, account, {}, definition, {}, {"private_argument"},
      time_from_string("2026-10-05 12:00:00"));
  }

  HttpResponse load(ReportingWebServlet& servlet,
      const WebPortalSession& session, const JsonValue& body) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/query_report_activities"),
      from<SharedBuffer>(to_string(body)));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report activity route.");
  }
}

TEST_SUITE("ReportActivity") {
  TEST_CASE("owner_and_status") {
    auto alice = DirectoryEntry::make_account(1, "Alice");
    auto bob = DirectoryEntry::make_account(2, "Bob");
    auto queued = make_job("queued", alice);
    auto running = make_job("running", alice);
    running.m_status = ReportJob::Status::RUNNING;
    running.m_modified = time_from_string("2026-10-06 00:00:00");
    auto failed = make_job("failed", alice);
    failed.m_status = ReportJob::Status::FAILED;
    failed.m_completed = time_from_string("2026-10-07 09:00:00");
    auto completed = make_job("completed", alice);
    completed.m_status = ReportJob::Status::COMPLETED;
    auto shared = make_job("shared", bob);
    shared.m_recipients = {alice};
    auto query = ReportActivityQuery();
    query.m_order = ReportActivityQuery::Order::ASCENDING;
    auto jobs = std::vector{completed, shared, failed, running, queued};
    auto page = query_report_activities(jobs, alice, query);
    REQUIRE(page.m_total_count == 3);
    REQUIRE(page.m_activities.size() == 3);
    REQUIRE(page.m_activities[0].m_id == "queued");
    REQUIRE(
      page.m_activities[0].m_status == ReportActivity::Status::GENERATING);
    REQUIRE(page.m_activities[1].m_id == "running");
    REQUIRE(page.m_activities[1].m_date_modified == running.m_modified->date());
    REQUIRE(page.m_activities[2].m_id == "failed");
    REQUIRE(page.m_activities[2].m_status == ReportActivity::Status::FAILED);
    REQUIRE(page.m_activities[2].m_date_modified == failed.m_completed.date());
    REQUIRE(query_report_activities(jobs, bob, query).m_total_count == 1);
    REQUIRE(query_report_activities(jobs,
      DirectoryEntry::make_account(3, "Other"), query).m_activities.empty());
  }

  TEST_CASE("ordering_and_pagination") {
    auto account = DirectoryEntry::make_account(1, "Alice");
    auto jobs = std::vector<ReportJob>();
    for(auto i = 0; i != 53; ++i) {
      auto job = make_job(std::to_string(i), account);
      job.m_created += seconds(i);
      jobs.push_back(job);
    }
    auto query = ReportActivityQuery();
    auto page = query_report_activities(jobs, account, query);
    REQUIRE(page.m_total_count == 53);
    REQUIRE(page.m_activities.size() == 50);
    REQUIRE(page.m_activities.front().m_id == "52");
    REQUIRE(page.m_activities.back().m_id == "3");
    query.m_page_index = 1;
    page = query_report_activities(jobs, account, query);
    REQUIRE(page.m_total_count == 53);
    REQUIRE(page.m_activities.size() == 3);
    REQUIRE(page.m_activities.front().m_id == "2");
    REQUIRE(page.m_activities.back().m_id == "0");
    query.m_page_index = std::numeric_limits<std::uint32_t>::max();
    page = query_report_activities(jobs, account, query);
    REQUIRE(page.m_total_count == 53);
    REQUIRE(page.m_activities.empty());
    jobs = {make_job("b", account), make_job("a", account)};
    jobs[0].m_definition.m_name = "Zulu";
    jobs[1].m_definition.m_name = "Alpha";
    jobs[0].m_status = ReportJob::Status::FAILED;
    auto count = 2;
    for(auto& job : jobs) {
      job.m_definition.m_parameters = {{"count", "Count", "Integer", true}};
      job.m_parameters["count"] = count;
      --count;
    }
    query.m_page_index = 0;
    for(auto column : {ReportActivityQuery::Column::TYPE,
        ReportActivityQuery::Column::PARAMETERS,
        ReportActivityQuery::Column::STATUS}) {
      query.m_column = column;
      query.m_order = ReportActivityQuery::Order::ASCENDING;
      REQUIRE(query_report_activities(
        jobs, account, query).m_activities.front().m_id == "a");
      query.m_order = ReportActivityQuery::Order::DESCENDING;
      REQUIRE(query_report_activities(
        jobs, account, query).m_activities.front().m_id == "b");
    }
    query.m_order = ReportActivityQuery::Order::NONE;
    REQUIRE(query_report_activities(
      jobs, account, query).m_activities.front().m_id == "a");
  }

  TEST_CASE("parameter_display") {
    auto account = DirectoryEntry::make_account(1, "Alice");
    auto job = make_job("job", account);
    job.m_definition.m_parameters = {
      {"account", "Account", "DirectoryEntry", true},
      {"accounts", "Accounts", "DirectoryEntryList", true},
      {"scope", "Scope", "Scope", true},
      {"currency", "Currency", "Currency", true},
      {"money", "Money", "Money", true},
      {"integer", "Integer", "Integer", true},
      {"decimal", "Decimal", "Decimal", true},
      {"date", "Date", "Date", true},
      {"time", "Time", "Time", true},
      {"timestamp", "Timestamp", "DateTime", true},
      {"range", "Range", "DateRange", true},
      {"optional", "Optional", "Integer", false}};
    job.m_parameters = get<JsonObject>(parse<JsonValue>(R"({
      "account": {"type":0,"id":1,"name":"Alice"},
      "accounts": [{"type":0,"id":1,"name":"Alice"},
        {"type":1,"id":2,"name":"Group"}],
      "scope": {"name":"","is_global":true,"countries":[],
        "venues":[],"tickers":[]},
      "currency":840,"integer":0,"decimal":1.25,
      "date":"20261005","time":"12:30:00",
      "timestamp":"20261005T123000",
      "range":{"start":"20261001","end":"20261005"},"optional":null
    })"));
    job.m_parameters["money"] = parse<JsonValue>(to_json(parse_money("12.34")));
    auto page = query_report_activities({job}, account, ReportActivityQuery());
    REQUIRE(page.m_activities.front().m_parameters == std::vector<std::string>({
      "Alice", "Alice, Group", "*", "USD", "12.34", "0", "1.25",
      "2026-10-05", "12:30:00", "2026-10-05T12:30:00",
      "2026-10-01 - 2026-10-05"}));
    test_round_trip_shuttle(page.m_activities.front(), [&] (const auto& value) {
      REQUIRE(value.m_id == "job");
      REQUIRE(value.m_type == "Example");
      REQUIRE(value.m_parameters == page.m_activities.front().m_parameters);
      REQUIRE(value.m_status == ReportActivity::Status::GENERATING);
      REQUIRE(value.m_date_modified == job.m_created.date());
    });
  }

  TEST_CASE("activity_endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local = LocalReportService({}, client,
      [] (const auto&, auto) { return 0; });
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = get<JsonObject>(
      parse<JsonValue>(R"({"sort":{"column":3,"order":0},"page_index":0})"));
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    auto job = make_job("mine", client.get_account());
    local.store(job);
    auto other = make_job("other", DirectoryEntry::make_account(123, "Other"));
    other.m_recipients = {client.get_account()};
    local.store(other);
    auto response = load(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto result = get<JsonObject>(parse<JsonValue>(response.get_body()));
    REQUIRE(result.at("status") == 1);
    REQUIRE(!get<bool>(result.at("is_empty")));
    REQUIRE(result.at("total_count") == 1);
    auto activities = get<std::vector<JsonValue>>(result.at("activities"));
    REQUIRE(activities.size() == 1);
    auto activity = get<JsonObject>(activities.front());
    REQUIRE(activity.at("id") == "mine");
    REQUIRE(activity.at("date_modified") == "20261005");
    REQUIRE(!activity.get("definition"));
    REQUIRE(!activity.get("arguments"));
    REQUIRE(!activity.get("error"));
    REQUIRE(!activity.get("account"));
    REQUIRE(!activity.get("recipients"));
    REQUIRE(to_string(response.get_body()).find("private_") ==
      std::string::npos);
    body["page_index"] = 1;
    result = get<JsonObject>(parse<JsonValue>(
      load(servlet, *session, body).get_body()));
    REQUIRE(!get<bool>(result.at("is_empty")));
    REQUIRE(get<std::vector<JsonValue>>(result.at("activities")).empty());
    for(auto invalid : {-1.0, 0.5,
        double(std::numeric_limits<std::uint32_t>::max()) + 1}) {
      body["page_index"] = invalid;
      REQUIRE(load(servlet, *session, body).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    body["page_index"] = 0;
    get<JsonObject>(body["sort"])["column"] = 4;
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    get<JsonObject>(body["sort"])["column"] = 0;
    get<JsonObject>(body["sort"])["order"] = 0.5;
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    get<JsonObject>(body["sort"])["order"] = 0;
    job.m_status = ReportJob::Status::COMPLETED;
    local.store(job);
    result = get<JsonObject>(
      parse<JsonValue>(load(servlet, *session, body).get_body()));
    REQUIRE(get<bool>(result.at("is_empty")));
    REQUIRE(result.at("total_count") == 0);
    local.close();
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
