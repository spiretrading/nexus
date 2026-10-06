#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  HttpResponse cancel(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(HttpMethod::POST,
      Uri("/api/reporting_service/cancel_report_jobs"),
      from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report cancellation route.");
  }
}

TEST_SUITE("ReportCancellation") {
  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1);
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto job = ReportJob("mine", client.get_account(), {}, {}, {}, {},
      time_from_string("2026-10-05 12:00:00"), {}, {},
      ReportJob::Status::FAILED);
    local.store(job);
    auto other = job;
    other.m_id = "other";
    other.m_account = DirectoryEntry::make_account(123, "Other");
    other.m_recipients = {client.get_account()};
    local.store(other);
    auto completed = job;
    completed.m_id = "completed";
    completed.m_status = ReportJob::Status::COMPLETED;
    local.store(completed);
    REQUIRE(cancel(servlet, *session,
      R"({"ids":["mine"]})").get_status_code() == HttpStatusCode::UNAUTHORIZED);
    REQUIRE(local.load_job("mine")->m_status == ReportJob::Status::FAILED);
    session->set_account(client.get_account());
    for(auto& body : {"", "{}", R"({"ids":1})", R"({"ids":[1]})"}) {
      REQUIRE(cancel(servlet, *session, body).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    for(auto& body : {R"({"ids":["mine","other"]})",
        R"({"ids":["mine","missing"]})"}) {
      REQUIRE(cancel(servlet, *session, body).get_status_code() ==
        HttpStatusCode::NOT_FOUND);
      REQUIRE(local.load_job("mine")->m_status == ReportJob::Status::FAILED);
    }
    auto response =
      cancel(servlet, *session, R"({"ids":["mine","mine","completed"]})");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    REQUIRE(local.load_job("mine")->m_status == ReportJob::Status::CANCELLED);
    REQUIRE(local.load_job("other")->m_status == ReportJob::Status::FAILED);
    REQUIRE(
      local.load_job("completed")->m_status == ReportJob::Status::COMPLETED);
    REQUIRE(local.query(
      client.get_account(), ReportActivityQuery()).m_total_count == 0);
    REQUIRE(cancel(servlet, *session,
      R"({"ids":["mine"]})").get_status_code() == HttpStatusCode::OK);
    REQUIRE(cancel(servlet, *session, R"({"ids":[]})").get_status_code() ==
      HttpStatusCode::OK);
    local.close();
    REQUIRE(cancel(servlet, *session, R"({"ids":[]})").get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }
}
