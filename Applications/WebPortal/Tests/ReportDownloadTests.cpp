#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/Tests/ReportingWebServletTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
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
    return dispatch(servlet, session, request);
  }
}

TEST_SUITE("ReportDownload") {

  TEST_CASE("filename_templates") {
    auto job = make_job(DirectoryEntry::make_account(1));
    job.m_created = time_from_string("2026-10-05 02:00:00");
    job.m_time_zone = "America/Toronto";
    REQUIRE(make_report_filename(job) == "example_2026-10-04.bin");
    job.m_definition.m_parameters = {
      {"period", "Period", "DateRange", true}};
    job.m_parameters["period"] = parse<JsonValue>(
      R"({"start":"20261001","end":"20261004"})");
    job.m_definition.m_output.m_filename =
      "Profit and Loss_{period.start}_{period.end}";
    auto name = "Profit and Loss_2026-10-01_2026-10-04.bin";
    REQUIRE(make_report_filename(job) == name);
    auto names = std::vector<std::string>{name,
      "Profit and Loss_2026-10-01_2026-10-04_2.bin"};
    REQUIRE(make_report_filename(job, names) ==
      "Profit and Loss_2026-10-01_2026-10-04_3.bin");
    job.m_definition.m_output.m_filename = "CON";
    REQUIRE(make_report_filename(job) == "_CON.bin");
    job.m_definition.m_output.m_filename = "../unsafe\\name:\r\n*? . ";
    REQUIRE(make_report_filename(job) == ".._unsafe_name_____.bin");
    job.m_definition.m_output.m_filename = "...";
    REQUIRE(make_report_filename(job) == "report.bin");
    job.m_definition.m_output.m_filename = std::string(300, 'x');
    REQUIRE(make_report_filename(job).string().size() == 164);
    job.m_definition.m_output.m_filename = "{missing}";
    REQUIRE_THROWS_AS(make_report_filename(job), std::runtime_error);
    job.m_definition.m_output.m_filename = "{period.start";
    REQUIRE_THROWS_AS(make_report_filename(job), std::runtime_error);
  }

  TEST_CASE("saved_filenames") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto local =
      LocalReportService({}, client, [] (const auto&, auto) { return 0; },
        TimeClient(std::in_place_type<LocalTimeClient>), 1,
        Timer(std::in_place_type<TriggerTimer>),
        Timer(std::in_place_type<TriggerTimer>));
    auto job = make_job(client.get_account());
    local.store(job);
    job.m_id = "second";
    local.store(job);
    REQUIRE(local.load_job(job.m_id)->m_filename ==
      "example_2026-10-05_2.bin");
    job.m_definition.m_output.m_filename = "Changed";
    local.store(job);
    REQUIRE(local.load_job(job.m_id)->m_filename ==
      "example_2026-10-05_2.bin");
    job.m_id = "third";
    job.m_definition.m_output.m_filename = "EXAMPLE_2026-10-05";
    local.store(job);
    REQUIRE(local.load_job(job.m_id)->m_filename ==
      "EXAMPLE_2026-10-05_3.bin");
  }

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
      "attachment; filename*=UTF-8''example_2026-10-05.bin");
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
    REQUIRE(file.m_name == std::filesystem::path("example_2026-10-05.bin"));
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
      "attachment; filename*=UTF-8''type____2026-10-05.csv");
    job.m_definition.m_id = "caf\xC3\xA9";
    job.m_id = "unicode";
    local.store(job);
    local.set_output(job.m_id, from<SharedBuffer>("a,b"));
    response = download(servlet, *session, "?id=unicode");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_header("Content-Disposition").has_value());
    REQUIRE(*response.get_header("Content-Disposition") ==
      "attachment; filename*=UTF-8''caf%C3%A9_2026-10-05.csv");
    for(auto& type : {"", "text/csv\r\nInjected: true"}) {
      job.m_definition.m_output.m_media_type = type;
      local.store(job);
      response = download(servlet, *session, "?id=unicode");
      REQUIRE(
        response.get_status_code() == HttpStatusCode::INTERNAL_SERVER_ERROR);
      REQUIRE(response.get_body().get_size() == 0);
    }
  }
}
