#include <barrier>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportJob make_job(const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Example", {}, {}, {}, {}, {},
      ReportOutputDefinition("text/csv", "csv"));
    return ReportJob("report", account, {}, definition, {}, {},
      time_from_string("2026-10-01 12:00:00"),
      time_from_string("2026-10-06 12:00:00"), {},
      ReportJob::Status::COMPLETED);
  }

  HttpResponse remove(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/delete_reports"), from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report deletion route.");
  }
}

TEST_SUITE("ReportDeletion") {
  TEST_CASE("endpoint_and_output_removal") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto group =
      client.make_directory("Group", DirectoryEntry::make_directory(0));
    auto recipient = client.make_account("Recipient", "", group);
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto job = make_job(client.get_account());
    job.m_recipients = {group};
    local.store(job);
    local.set_output(job.m_id, from<SharedBuffer>("a,b"));
    REQUIRE(local.load_report(recipient, job.m_id).m_id == job.m_id);
    auto untouched = job;
    untouched.m_id = "untouched";
    local.store(untouched);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto body = std::string(R"({"ids":["report","report"]})");
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    REQUIRE(local.load_job(job.m_id));
    session->set_account(recipient);
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    REQUIRE(local.load_job(job.m_id));
    session = sessions.create();
    session->set_account(client.get_account());
    for(auto& invalid : {"", "{", "{}", R"({"ids":null})", R"({"ids":1})",
        R"({"ids":[1]})"}) {
      REQUIRE(remove(servlet, *session, invalid).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(remove(servlet, *session, R"({"ids":[]})").get_status_code() ==
      HttpStatusCode::OK);
    auto response = remove(servlet, *session, body);
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    REQUIRE(!local.load_job(job.m_id));
    REQUIRE(local.load_job(untouched.m_id));
    for(auto& account : {client.get_account(), recipient}) {
      auto page = local.query(account, GeneratedReportQuery());
      REQUIRE(page.m_reports.size() == 1);
      REQUIRE(page.m_reports[0].m_id == untouched.m_id);
      REQUIRE_THROWS_AS(
        local.load_report(account, job.m_id), ReportNotFoundException);
      REQUIRE_THROWS_AS(
        local.load_file(account, job.m_id), ReportNotFoundException);
    }
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    local.store(job);
    REQUIRE_THROWS_AS(
      local.load_file(job.m_account, job.m_id), ReportNotFoundException);
    local.close();
    REQUIRE(remove(servlet, *session, body).get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

  TEST_CASE("ownership_and_completion_before_deletion") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto service = ReportService(&local);
    auto job = make_job(client.get_account());
    local.store(job);
    auto other = job;
    other.m_id = "other";
    other.m_account = DirectoryEntry::make_account(123, "Other");
    other.m_recipients = {client.get_account()};
    local.store(other);
    for(auto& id : {"missing", "other"}) {
      REQUIRE_THROWS_AS(service.remove(job.m_account, {job.m_id, id}),
        ReportNotFoundException);
      REQUIRE(local.load_job(job.m_id));
      REQUIRE(local.load_job(other.m_id));
    }
    other.m_account = job.m_account;
    for(auto status : {ReportJob::Status::QUEUED, ReportJob::Status::RUNNING,
        ReportJob::Status::FAILED, ReportJob::Status::CANCELLED}) {
      other.m_status = status;
      local.store(other);
      REQUIRE_THROWS_AS(service.remove(job.m_account, {job.m_id, other.m_id}),
        ReportNotFoundException);
      REQUIRE(local.load_job(job.m_id));
      REQUIRE(local.load_job(other.m_id)->m_status == status);
    }
    other.m_status = ReportJob::Status::COMPLETED;
    local.store(other);
    service.remove(job.m_account, {job.m_id, other.m_id});
    REQUIRE(local.load_jobs().empty());
  }

  TEST_CASE("concurrent_share_does_not_restore_deleted_job") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto recipient = client.make_account(
      "Recipient", "", DirectoryEntry::make_directory(0));
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto job = make_job(client.get_account());
    local.store(job);
    auto ready = std::barrier(3);
    auto share_error = std::exception_ptr();
    auto remove_error = std::exception_ptr();
    auto sharing = std::jthread([&] {
      ready.arrive_and_wait();
      try {
        local.share(job.m_account, {job.m_id}, {recipient});
      } catch(const ReportNotFoundException&) {
      } catch(const std::exception&) {
        share_error = std::current_exception();
      }
    });
    auto deleting = std::jthread([&] {
      ready.arrive_and_wait();
      try {
        local.remove(job.m_account, {job.m_id});
      } catch(const std::exception&) {
        remove_error = std::current_exception();
      }
    });
    ready.arrive_and_wait();
    sharing.join();
    deleting.join();
    REQUIRE(!share_error);
    REQUIRE(!remove_error);
    REQUIRE(!local.load_job(job.m_id));
  }
}
