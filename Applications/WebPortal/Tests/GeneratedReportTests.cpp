#include <limits>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
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
  ReportJob make_job(std::string id, const DirectoryEntry& account) {
    auto parameters = JsonObject();
    parameters["account"] = parse<JsonValue>(to_json(account));
    auto definition = ReportDefinition("example", "Example", {}, {},
      {{"account", "Account", "DirectoryEntry", true}}, "private_command");
    return ReportJob(std::move(id), account, {}, std::move(definition),
      std::move(parameters), {"private_argument"},
      time_from_string("2026-10-01 12:00:00"),
      time_from_string("2026-10-05 13:00:00"), std::nullopt,
      ReportJob::Status::COMPLETED);
  }

  HttpResponse load(ReportingWebServlet& servlet,
      const WebPortalSession& session, const JsonValue& body) {
    auto request = HttpRequest(
      HttpMethod::POST, Uri("/api/reporting_service/query_generated_reports"),
      from<SharedBuffer>(to_string(body)));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing generated report route.");
  }
}

TEST_SUITE("GeneratedReport") {
  TEST_CASE("ownership_and_sharing") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto root = DirectoryEntry::make_directory(0);
    auto group = client.make_directory("Group", root);
    auto subgroup = client.make_directory("Subgroup", group);
    auto alice = client.make_account("Alice", "", subgroup);
    auto bob = client.make_account("Bob", "", root);
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto owned = make_job("owned", alice);
    local.store(owned);
    auto direct = make_job("direct", bob);
    direct.m_recipients = {alice};
    local.store(direct);
    auto shared = make_job("group", bob);
    shared.m_recipients = {group, subgroup};
    local.store(shared);
    local.store(make_job("private", bob));
    auto global = make_job("global", bob);
    global.m_recipients = {DirectoryEntry::STAR_DIRECTORY};
    local.store(global);
    for(auto status : {ReportJob::Status::QUEUED, ReportJob::Status::RUNNING,
        ReportJob::Status::FAILED, ReportJob::Status::CANCELLED}) {
      auto pending = direct;
      pending.m_id = std::to_string(static_cast<int>(status));
      pending.m_status = status;
      local.store(pending);
    }
    auto service = ReportService(&local);
    auto page = service.query(alice, GeneratedReportQuery());
    REQUIRE(!page.m_is_empty);
    REQUIRE(page.m_filtered_count == 3);
    REQUIRE(page.m_reports.size() == 3);
    REQUIRE(page.m_reports[0].m_id == "direct");
    REQUIRE(page.m_reports[1].m_id == "group");
    REQUIRE(page.m_reports[2].m_id == "owned");
    REQUIRE(
      service.query(client.get_account(), GeneratedReportQuery()).m_is_empty);
    client.associate(alice, root);
    client.detach(alice, subgroup);
    page = service.query(alice, GeneratedReportQuery());
    REQUIRE(page.m_filtered_count == 2);
    REQUIRE(page.m_reports[0].m_id == "direct");
    REQUIRE(page.m_reports[1].m_id == "owned");
    service.close();
    REQUIRE_THROWS_AS(
      service.query(alice, GeneratedReportQuery()), std::runtime_error);
  }

  TEST_CASE("filters") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto account = client.get_account();
    auto job = make_job("report", account);
    auto jobs = std::vector{job};
    auto query = GeneratedReportQuery();
    for(auto& text : {"  aMp  ", "oct 05, 2026", "ROOT"}) {
      query.m_query = text;
      auto page = query_generated_reports(jobs, account, query, client);
      REQUIRE(page.m_filtered_count == 1);
      REQUIRE(page.m_reports.front().m_date_created == job.m_completed.date());
    }
    query.m_query = "unmatched";
    auto page = query_generated_reports(jobs, account, query, client);
    REQUIRE(!page.m_is_empty);
    REQUIRE(page.m_filtered_count == 0);
    REQUIRE(page.m_reports.empty());
    query.m_query.clear();
    query.m_start_date = from_undelimited_string("20261005");
    query.m_end_date = query.m_start_date;
    REQUIRE(query_generated_reports(
      jobs, account, query, client).m_filtered_count == 1);
    query.m_start_date = from_undelimited_string("20261006");
    query.m_end_date.reset();
    REQUIRE(query_generated_reports(
      jobs, account, query, client).m_filtered_count == 0);
    query.m_start_date.reset();
    query.m_end_date = from_undelimited_string("20261004");
    REQUIRE(query_generated_reports(
      jobs, account, query, client).m_filtered_count == 0);
    query.m_start_date = from_undelimited_string("20261006");
    REQUIRE_THROWS_AS(query_generated_reports(jobs, account, query, client),
      std::invalid_argument);
    query.m_start_date.reset();
    query.m_end_date = date();
    REQUIRE_THROWS_AS(query_generated_reports(jobs, account, query, client),
      std::invalid_argument);
    query.m_end_date.reset();
    REQUIRE(query_generated_reports({}, account, query, client).m_is_empty);
    jobs.front().m_status = ReportJob::Status::FAILED;
    REQUIRE(query_generated_reports(jobs, account, query, client).m_is_empty);
  }

  TEST_CASE("ordering_and_pagination") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto account = client.get_account();
    auto jobs = std::vector<ReportJob>();
    for(auto i = 0; i != 53; ++i) {
      auto job = make_job(std::to_string(i), account);
      job.m_completed += seconds(i);
      jobs.push_back(job);
    }
    auto query = GeneratedReportQuery();
    auto page = query_generated_reports(jobs, account, query, client);
    REQUIRE(page.m_filtered_count == 53);
    REQUIRE(page.m_reports.size() == 50);
    REQUIRE(page.m_reports.front().m_id == "52");
    REQUIRE(page.m_reports.back().m_id == "3");
    query.m_page_index = 1;
    page = query_generated_reports(jobs, account, query, client);
    REQUIRE(page.m_filtered_count == 53);
    REQUIRE(page.m_reports.size() == 3);
    REQUIRE(page.m_reports.front().m_id == "2");
    query.m_page_index = std::numeric_limits<std::uint32_t>::max();
    page = query_generated_reports(jobs, account, query, client);
    REQUIRE(!page.m_is_empty);
    REQUIRE(page.m_filtered_count == 53);
    REQUIRE(page.m_reports.empty());
    jobs = {make_job("b", account), make_job("a", account)};
    jobs[0].m_definition.m_name = "Zulu";
    jobs[1].m_definition.m_name = "Alpha";
    jobs[0].m_parameters["account"] =
      parse<JsonValue>(to_json(DirectoryEntry::make_account(2, "Zulu")));
    jobs[1].m_parameters["account"] =
      parse<JsonValue>(to_json(DirectoryEntry::make_account(3, "Alpha")));
    jobs[0].m_completed += days(1);
    query.m_page_index = 0;
    for(auto column : {GeneratedReportQuery::Column::TYPE,
        GeneratedReportQuery::Column::PARAMETERS,
        GeneratedReportQuery::Column::DATE_CREATED}) {
      query.m_column = column;
      query.m_order = GeneratedReportQuery::Order::ASCENDING;
      REQUIRE(query_generated_reports(
        jobs, account, query, client).m_reports.front().m_id == "a");
      query.m_order = GeneratedReportQuery::Order::DESCENDING;
      REQUIRE(query_generated_reports(
        jobs, account, query, client).m_reports.front().m_id == "b");
    }
    jobs[0].m_completed = jobs[1].m_completed;
    query.m_order = GeneratedReportQuery::Order::NONE;
    REQUIRE(query_generated_reports(
      jobs, account, query, client).m_reports.front().m_id == "a");
  }

  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = get<JsonObject>(parse<JsonValue>(R"({"filters":{
      "query":"","start_date":null,"end_date":null},
      "sort":{"column":2,"order":0},"page_index":0})"));
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    auto job = make_job("report /?", client.get_account());
    local.store(job);
    local.store(
      make_job("private", DirectoryEntry::make_account(123, "Other")));
    auto response = load(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    auto result = get<JsonObject>(parse<JsonValue>(response.get_body()));
    REQUIRE(result.at("status") == 1);
    REQUIRE(!get<bool>(result.at("is_empty")));
    REQUIRE(result.at("filtered_count") == 1);
    auto reports = get<std::vector<JsonValue>>(result.at("reports"));
    REQUIRE(reports.size() == 1);
    auto report = get<JsonObject>(reports.front());
    REQUIRE(report.at("id") == job.m_id);
    REQUIRE(report.at("type") == "Example");
    REQUIRE(report.at("url") == "/reports/report%20%2F%3F");
    REQUIRE(report.at("date_created") == "20261005");
    REQUIRE(!report.get("definition"));
    REQUIRE(!report.get("arguments"));
    REQUIRE(!report.get("account"));
    REQUIRE(!report.get("recipients"));
    REQUIRE(!report.get("error"));
    REQUIRE(
      to_string(response.get_body()).find("private") == std::string::npos);
    auto value = from_json<GeneratedReport>(reports.front());
    test_round_trip_shuttle(value, [&] (const auto& received) {
      REQUIRE(received.m_id == value.m_id);
      REQUIRE(received.m_type == value.m_type);
      REQUIRE(received.m_parameters == value.m_parameters);
      REQUIRE(to_string(received.m_url) == to_string(value.m_url));
      REQUIRE(received.m_date_created == value.m_date_created);
    });
    auto& filters = get<JsonObject>(body["filters"]);
    filters["query"] = std::string("unmatched");
    CAPTURE(to_string(JsonValue(body)));
    response = load(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    CAPTURE(to_string(response.get_body()));
    result = get<JsonObject>(parse<JsonValue>(response.get_body()));
    REQUIRE(!get<bool>(result.at("is_empty")));
    REQUIRE(result.at("filtered_count") == 0);
    filters["query"] = std::string();
    filters["start_date"] = std::string("20261005");
    filters["end_date"] = std::string("20261005");
    REQUIRE(
      load(servlet, *session, body).get_status_code() == HttpStatusCode::OK);
    for(auto& invalid : {"20260230", "invalid", "+infinity",
        "not-a-date-time", "20261006", "20261005junk", "2026105", ""}) {
      filters["start_date"] = std::string(invalid);
      REQUIRE(load(servlet, *session, body).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    filters["start_date"] = JsonNull();
    for(auto invalid : {-1.0, 0.5,
        double(std::numeric_limits<std::uint32_t>::max()) + 1}) {
      body["page_index"] = invalid;
      REQUIRE(load(servlet, *session, body).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    body["page_index"] = 0;
    get<JsonObject>(body["sort"])["column"] = 3;
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    get<JsonObject>(body["sort"])["column"] = 0;
    get<JsonObject>(body["sort"])["order"] = 0.5;
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    get<JsonObject>(body["sort"])["order"] = 0;
    local.close();
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
