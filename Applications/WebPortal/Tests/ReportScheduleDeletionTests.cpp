#include <barrier>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportSchedule make_schedule(const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Example", {}, {"*"},
      {{"count", "Count", "Integer", true}}, "report_program", {"{count}"},
      ReportOutputDefinition("text/csv", "csv"));
    auto parameters = JsonObject();
    parameters["count"] = 1;
    return ReportSchedule("schedule", account, definition, parameters, {},
      time_from_string("2026-10-01 12:00:00"),
      time_from_string("2026-10-07 09:00:00"),
      time_from_string("2026-10-07 09:00:00"), {}, "America/Toronto");
  }

  HttpResponse remove(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(
      HttpMethod::POST, Uri("/api/reporting_service/delete_scheduled_report"),
      from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing schedule deletion route.");
  }
}

TEST_SUITE("ReportScheduleDeletion") {
  TEST_CASE("owner_only_deletion_and_existing_reports") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto group =
      client.make_directory("Group", DirectoryEntry::make_directory(0));
    auto recipient = client.make_account("Recipient", "", group);
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; });
    auto service = ReportService(&local);
    auto schedule = make_schedule(client.get_account());
    schedule.m_recipients = {group, recipient};
    service.store(schedule);
    auto other = schedule;
    other.m_id = "other";
    service.store(other);
    auto job = ReportJob(schedule.m_id, schedule.m_account, {},
      schedule.m_definition, schedule.m_parameters, {}, schedule.m_created,
      schedule.m_created, {}, ReportJob::Status::COMPLETED);
    local.store(job);
    local.set_output(job.m_id, from<SharedBuffer>("value\n1"));
    for(auto& account : {group, recipient, DirectoryEntry(),
        DirectoryEntry::make_directory(schedule.m_account.m_id)}) {
      REQUIRE_THROWS_AS(service.remove_schedule(account, schedule.m_id),
        ReportNotFoundException);
      REQUIRE(local.load_schedules().size() == 2);
    }
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = std::string(R"({"id":"schedule"})");
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(recipient);
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    REQUIRE(local.load_schedules().size() == 2);
    session = sessions.create();
    session->set_account(schedule.m_account);
    for(auto& invalid : {"", "{", "{}", "[]", R"({"id":null})",
        R"({"id":1})", R"({"id":""})"}) {
      REQUIRE(remove(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(remove(servlet, *session, R"({"id":"missing"})").
      get_status_code() == HttpStatusCode::NOT_FOUND);
    auto response = remove(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    REQUIRE_THROWS_AS(service.load_schedule(schedule.m_account, schedule.m_id),
      ReportNotFoundException);
    auto page = service.query(schedule.m_account, ScheduledReportQuery());
    REQUIRE(page.m_filtered_count == 1);
    REQUIRE(page.m_schedules.size() == 1);
    REQUIRE(page.m_schedules[0].m_id == other.m_id);
    REQUIRE(to_json(service.load_schedule(other.m_account, other.m_id)) ==
      to_json(other));
    auto saved = local.load_job(job.m_id);
    REQUIRE(saved);
    REQUIRE(to_json(*saved) == to_json(job));
    REQUIRE(to_string(service.load_file(job.m_account, job.m_id).m_content) ==
      "value\n1");
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    other.m_repeat_interval =
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::DAY);
    service.store(other);
    service.remove_schedule(other.m_account, other.m_id);
    REQUIRE(
      service.query(schedule.m_account, ScheduledReportQuery()).m_is_empty);
    local.close();
    REQUIRE_THROWS_AS(
      service.remove_schedule(schedule.m_account, schedule.m_id),
      EndOfFileException);
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

  TEST_CASE("concurrent_update_does_not_restore_deleted_schedule") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto schedule = make_schedule(client.get_account());
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto local = LocalReportService({schedule.m_definition}, client,
      [] (const auto&, auto) { return 0; }, &time);
    local.store(schedule);
    auto parameters = JsonObject();
    parameters["count"] = 2;
    auto submission = ReportScheduleSubmission(
      ReportSubmission("example", parameters, {}), schedule.m_start_time,
      schedule.m_repeat_interval, schedule.m_time_zone);
    auto ready = std::barrier(3);
    auto update_error = std::exception_ptr();
    auto updating = std::jthread([&] {
      ready.arrive_and_wait();
      try {
        local.update_schedule(schedule.m_account, schedule.m_id, submission);
      } catch(const ReportNotFoundException&) {
      } catch(const std::exception&) {
        update_error = std::current_exception();
      }
    });
    auto remove_error = std::exception_ptr();
    auto deleting = std::jthread([&] {
      ready.arrive_and_wait();
      try {
        local.remove_schedule(schedule.m_account, schedule.m_id);
      } catch(const std::exception&) {
        remove_error = std::current_exception();
      }
    });
    ready.arrive_and_wait();
    updating.join();
    deleting.join();
    REQUIRE(!update_error);
    REQUIRE(!remove_error);
    REQUIRE(local.load_schedules().empty());
    REQUIRE_THROWS_AS(local.update_schedule(
      schedule.m_account, schedule.m_id, submission), ReportNotFoundException);
  }
}
