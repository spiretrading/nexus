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
using namespace std::string_literals;

namespace {
  ReportJob make_job(const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Example", {}, {}, {},
      "private_command", {}, ReportOutputDefinition(
        "application/octet-stream", "bin"));
    return ReportJob("report", account, {}, std::move(definition), {}, {},
      time_from_string("2026-10-05 12:00:00"),
      time_from_string("2026-10-05 12:01:00"), {},
      ReportJob::Status::COMPLETED);
  }

  HttpResponse download(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& query) {
    auto request = HttpRequest(
      HttpMethod::GET, Uri("/api/reporting_service/download_report" + query));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report download route.");
  }
}

TEST_SUITE("ReportDownload") {
  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto job = make_job(client.get_account());
    local.store(job);
    auto bytes = "A\0B\r\n"s;
    local.set_output(job.m_id, SharedBuffer(bytes.data(), bytes.size()));
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    REQUIRE(download(servlet, *session, "?id=report").get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    for(auto& query : {"", "?other=report", "?id=", "?id=report&id=report"}) {
      REQUIRE(download(servlet, *session, query).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    REQUIRE(download(servlet, *session, "?id=missing").get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    auto response = download(servlet, *session, "?id=report");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(to_string(response.get_body()) == bytes);
    REQUIRE(response.get_header("Content-Length").has_value());
    REQUIRE(
      *response.get_header("Content-Length") == std::to_string(bytes.size()));
    REQUIRE(response.get_header("Content-Type").has_value());
    REQUIRE(*response.get_header("Content-Type") == "application/octet-stream");
    REQUIRE(response.get_header("Content-Disposition").has_value());
    REQUIRE(*response.get_header("Content-Disposition") ==
      "attachment; filename*=UTF-8''example-report.bin");
    REQUIRE(response.get_header("Cache-Control").has_value());
    REQUIRE(*response.get_header("Cache-Control") == "private, no-store");
    REQUIRE(response.get_header("X-Content-Type-Options").has_value());
    REQUIRE(*response.get_header("X-Content-Type-Options") == "nosniff");
    local.set_output(job.m_id, SharedBuffer());
    response = download(servlet, *session, "?id=report");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    auto missing = job;
    missing.m_id = "no-output";
    local.store(missing);
    REQUIRE(download(servlet, *session, "?id=no-output").get_status_code() ==
      HttpStatusCode::NOT_FOUND);
    local.close();
    REQUIRE(download(servlet, *session, "?id=report").get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

  TEST_CASE("sharing_and_status") {
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
    auto job = make_job(bob);
    job.m_recipients = {group};
    local.store(job);
    local.set_output(job.m_id, from<SharedBuffer>("output"));
    auto service = ReportService(&local);
    REQUIRE(
      to_string(service.load_file(alice, job.m_id).m_content) == "output");
    REQUIRE(to_string(service.load_file(bob, job.m_id).m_content) == "output");
    REQUIRE_THROWS_AS(service.load_file(client.get_account(), job.m_id),
      ReportNotFoundException);
    client.associate(alice, root);
    client.detach(alice, subgroup);
    REQUIRE_THROWS_AS(
      service.load_file(alice, job.m_id), ReportNotFoundException);
    job.m_recipients = {alice};
    local.store(job);
    REQUIRE(
      to_string(service.load_file(alice, job.m_id).m_content) == "output");
    for(auto status : {ReportJob::Status::QUEUED, ReportJob::Status::RUNNING,
        ReportJob::Status::FAILED, ReportJob::Status::CANCELLED}) {
      job.m_status = status;
      local.store(job);
      REQUIRE_THROWS_AS(
        service.load_file(alice, job.m_id), ReportNotFoundException);
      REQUIRE_THROWS_AS(
        service.load_file(bob, job.m_id), ReportNotFoundException);
    }
    job.m_status = ReportJob::Status::COMPLETED;
    job.m_recipients = {DirectoryEntry::STAR_DIRECTORY};
    local.store(job);
    REQUIRE_THROWS_AS(
      service.load_file(alice, job.m_id), ReportNotFoundException);
    job.m_recipients.clear();
    local.store(job);
    REQUIRE_THROWS_AS(
      service.load_file(alice, job.m_id), ReportNotFoundException);
    REQUIRE_THROWS_AS(
      service.load_file(group, job.m_id), ReportNotFoundException);
  }

  TEST_CASE("output_snapshots") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto job = make_job(client.get_account());
    local.store(job);
    auto content = from<SharedBuffer>("original");
    local.set_output(job.m_id, content);
    content.get_mutable_data()[0] = 'X';
    auto file = local.load_file(job.m_account, job.m_id);
    REQUIRE(file.m_name == std::filesystem::path("example-report.bin"));
    REQUIRE(file.m_media_type == "application/octet-stream");
    REQUIRE(to_string(file.m_content) == "original");
    file.m_content.get_mutable_data()[0] = 'Y';
    REQUIRE(to_string(local.load_file(job.m_account, job.m_id).m_content) ==
      "original");
    local.close();
    REQUIRE_THROWS_AS(local.set_output(job.m_id, content), std::runtime_error);
  }

  TEST_CASE("headers") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto job = make_job(client.get_account());
    job.m_id = "report /?";
    job.m_definition.m_id = "type\"\r\n";
    job.m_definition.m_output = ReportOutputDefinition("text/csv", "csv");
    local.store(job);
    local.set_output(job.m_id, from<SharedBuffer>("a,b"));
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    session->set_account(client.get_account());
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    auto response = download(servlet, *session, "?id=report%20%2F%3F");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_header("Content-Disposition").has_value());
    REQUIRE(*response.get_header("Content-Disposition") ==
      "attachment; filename*=UTF-8''type%22%0D%0A-report%20%2F%3F.csv");
    job.m_definition.m_id = "caf\xC3\xA9";
    local.store(job);
    response = download(servlet, *session, "?id=report%20%2F%3F");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_header("Content-Disposition").has_value());
    REQUIRE(*response.get_header("Content-Disposition") ==
      "attachment; filename*=UTF-8''caf%C3%A9-report%20%2F%3F.csv");
    for(auto& type : {"", "text/csv\r\nInjected: true"}) {
      job.m_definition.m_output.m_media_type = type;
      local.store(job);
      response = download(servlet, *session, "?id=report%20%2F%3F");
      REQUIRE(
        response.get_status_code() == HttpStatusCode::INTERNAL_SERVER_ERROR);
      REQUIRE(response.get_body().get_size() == 0);
    }
  }
}
