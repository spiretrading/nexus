#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  HttpResponse run(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/run_scheduled_report"),
      from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing schedule execution route.");
  }
}

TEST_SUITE("ReportScheduleRun") {
  TEST_CASE("owned_schedule_execution_and_validation") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto recipient =
      client.make_account("Recipient", "", DirectoryEntry::make_directory(0));
    auto definition = ReportDefinition("example", "Example", {}, {"*"},
      {{"count", "Count", "Integer", true}}, "original", {"{count}"});
    auto parameters = JsonObject();
    parameters["count"] = 5;
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto schedule = ReportSchedule("schedule", client.get_account(), definition,
      parameters, {recipient}, time.get_time(),
      time_from_string("2026-01-01 09:00:00"),
      time_from_string("2026-01-01 09:00:00"), {}, "America/Toronto");
    definition.m_command = "updated";
    auto started = Queue<ReportJob>();
    auto local = LocalReportService({definition}, client,
      [&] (const auto& job, auto) { started.push(job); return 0; }, &time, 2,
      Timer(std::in_place_type<TriggerTimer>));
    local.store(schedule);
    auto service = ReportService(&local);
    auto id = run_schedule(service, schedule.m_account, schedule.m_id);
    auto job = started.pop();
    REQUIRE(job.m_id == id);
    REQUIRE(job.m_id != schedule.m_id);
    REQUIRE(job.m_account == schedule.m_account);
    REQUIRE(job.m_recipients == schedule.m_recipients);
    REQUIRE(job.m_parameters == parameters);
    REQUIRE(job.m_definition.m_command == "updated");
    REQUIRE(job.m_arguments == std::vector<std::string>({"5"}));
    REQUIRE(job.m_created == time.get_time());
    REQUIRE(to_json(local.load_schedule(schedule.m_account, schedule.m_id)) ==
      to_json(schedule));
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = std::string(R"({"id":"schedule"})");
    REQUIRE(run(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(recipient);
    REQUIRE(run(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    session = sessions.create();
    session->set_account(schedule.m_account);
    for(auto& invalid : {"", "{", "{}", "[]", R"({"id":null})",
        R"({"id":1})", R"({"id":""})"}) {
      REQUIRE(run(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(run(servlet, *session, R"({"id":"missing"})").get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    schedule.m_repeat_interval =
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::MONTH);
    local.store(schedule);
    auto response = run(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    REQUIRE(started.pop().m_id != id);
    REQUIRE(to_json(local.load_schedule(schedule.m_account, schedule.m_id)) ==
      to_json(schedule));
    local.set_definitions({});
    REQUIRE(run(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    local.set_definitions({definition});
    schedule.m_parameters["count"] = 1.5;
    local.store(schedule);
    REQUIRE(run(servlet, *session, body).get_status_code() ==
      HttpStatusCode::BAD_REQUEST);
    REQUIRE(local.load_jobs().size() == 2);
    local.close();
    REQUIRE(run(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
