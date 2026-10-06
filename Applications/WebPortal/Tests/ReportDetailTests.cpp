#include <Beam/Json/JsonParser.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
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
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportJob make_job(const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Saved Title", {}, {"*"},
      {{"count", "Saved Count", "Integer", true},
        {"missing", "Missing", "String", false},
        {"omitted", "Omitted", "String", false},
        {"currency", "Currency", "Currency", true},
        {"period", "Period", "DateRange", true}},
      "private_command", {}, ReportOutputDefinition("text/csv", "csv"));
    auto parameters = JsonObject();
    parameters["count"] = 0;
    parameters["omitted"] = JsonNull();
    parameters["currency"] = 840;
    parameters["period"] =
      parse<JsonValue>(R"({"start":"20261001","end":null})");
    return ReportJob("report /?", account, {}, std::move(definition),
      std::move(parameters), {"private_argument"},
      time_from_string("2026-10-01 12:00:00"),
      time_from_string("2026-10-06 12:00:00"), {}, ReportJob::Status::COMPLETED,
      0, "private_diagnostic");
  }

  HttpResponse load(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/load_report"), from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report detail route.");
  }
}

TEST_SUITE("ReportDetail") {
  TEST_CASE("endpoint_and_saved_metadata") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>));
    auto job = make_job(client.get_account());
    local.store(job);
    auto definition = job.m_definition;
    definition.m_name = "New Title";
    definition.m_parameters[0].m_label = "New Count";
    local.set_definitions({definition});
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = std::string(R"({"id":"report /?"})");
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    for(auto& invalid : {"", "{", "[]", "{}", R"({"id":null})",
        R"({"id":42})", R"({"id":""})"}) {
      REQUIRE(load(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(load(servlet, *session, R"({"id":"missing"})").get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    auto response = load(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_header("Cache-Control").has_value());
    REQUIRE(*response.get_header("Cache-Control") == "private, no-store");
    REQUIRE(response.get_header("Content-Type").has_value());
    REQUIRE(*response.get_header("Content-Type") == "application/json");
    auto text = to_string(response.get_body());
    REQUIRE(text.find("private") == std::string::npos);
    auto report = from_json<ReportDetail>(text);
    REQUIRE(report.m_id == job.m_id);
    REQUIRE(report.m_title == "Saved Title");
    REQUIRE(report.m_parameters.size() == 3);
    REQUIRE(report.m_parameters[0].m_label == "Saved Count");
    REQUIRE(report.m_parameters[0].m_value == "0");
    REQUIRE(report.m_parameters[1].m_label == "Currency");
    REQUIRE(report.m_parameters[1].m_value == "USD");
    REQUIRE(report.m_parameters[2].m_label == "Period");
    REQUIRE(report.m_parameters[2].m_value == "2026-10-01 - ");
    REQUIRE(to_string(report.m_file_path) ==
      "/api/reporting_service/download_report?id=report%20%2F%3F");
    test_round_trip_shuttle(report, [&] (const auto& received) {
      REQUIRE(to_json(received) == to_json(report));
    });
    local.set_definitions({});
    REQUIRE(to_string(load(servlet, *session, body).get_body()) == text);
    local.close();
    REQUIRE(load(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

  TEST_CASE("ownership_sharing_and_status") {
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
        Timer(std::in_place_type<TriggerTimer>));
    auto service = ReportService(&local);
    auto job = make_job(bob);
    job.m_recipients = {group};
    local.store(job);
    REQUIRE(service.load_report(bob, job.m_id).m_id == job.m_id);
    REQUIRE(service.load_report(alice, job.m_id).m_id == job.m_id);
    REQUIRE_THROWS_AS(service.load_report(client.get_account(), job.m_id),
      ReportNotFoundException);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    session->set_account(client.get_account());
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto response = load(servlet, *session, R"({"id":"report /?"})");
    REQUIRE(response.get_status_code() == HttpStatusCode::NOT_FOUND);
    client.associate(alice, root);
    client.detach(alice, subgroup);
    REQUIRE_THROWS_AS(
      service.load_report(alice, job.m_id), ReportNotFoundException);
    job.m_recipients = {alice};
    local.store(job);
    REQUIRE(service.load_report(alice, job.m_id).m_id == job.m_id);
    for(auto status : {ReportJob::Status::QUEUED, ReportJob::Status::RUNNING,
        ReportJob::Status::FAILED, ReportJob::Status::CANCELLED}) {
      job.m_status = status;
      local.store(job);
      REQUIRE_THROWS_AS(
        service.load_report(alice, job.m_id), ReportNotFoundException);
      REQUIRE_THROWS_AS(
        service.load_report(bob, job.m_id), ReportNotFoundException);
    }
    job.m_status = ReportJob::Status::COMPLETED;
    job.m_recipients = {DirectoryEntry::STAR_DIRECTORY};
    local.store(job);
    REQUIRE_THROWS_AS(
      service.load_report(alice, job.m_id), ReportNotFoundException);
    REQUIRE_THROWS_AS(
      service.load_report(group, job.m_id), ReportNotFoundException);
  }
}
