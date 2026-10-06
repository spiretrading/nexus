#include <atomic>
#include <semaphore>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <doctest/doctest.h>
#include "WebPortal/LocalReportService.hpp"
#include "WebPortal/ReportingWebServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  ReportJob make_job(const DirectoryEntry& account) {
    auto definition =
      ReportDefinition("example", {}, {}, {"*"}, {}, "saved_program");
    return ReportJob("mine", account, {}, definition, {}, {"saved_argument"},
      time_from_string("2026-10-05 12:00:00"),
      time_from_string("2026-10-05 12:01:00"), {}, ReportJob::Status::FAILED, 7,
      "Failed.");
  }

  HttpResponse retry(ReportingWebServlet& servlet,
      const WebPortalSession& session, const std::string& body) {
    auto request = HttpRequest(
      HttpMethod::POST, Uri("/api/reporting_service/retry_report_jobs"),
      from<SharedBuffer>(body));
    request.add(Cookie("sessionid", session.get_id()));
    for(auto& slot : servlet.get_slots()) {
      if(slot.m_predicate(request)) {
        return slot.m_slot(request);
      }
    }
    throw std::runtime_error("Missing report retry route.");
  }
}

TEST_SUITE("ReportRetry") {
  TEST_CASE("endpoint") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto job = make_job(client.get_account());
    auto definition = job.m_definition;
    definition.m_command = "updated_program";
    auto executions = Queue<ReportJob>();
    auto release = std::binary_semaphore(0);
    auto calls = std::atomic_int(0);
    auto local = LocalReportService({definition}, client,
      [&] (const auto& value, auto stop) {
        ++calls;
        auto cancel = std::stop_callback(stop, [&] { release.release(); });
        executions.push(value);
        release.acquire();
        return 0;
      });
    local.store(job);
    auto other = job;
    other.m_id = "other";
    other.m_account = DirectoryEntry::make_account(123, "Other");
    other.m_recipients = {client.get_account()};
    local.store(other);
    auto unchanged = std::vector<ReportJob>();
    for(auto status : {ReportJob::Status::QUEUED, ReportJob::Status::RUNNING,
        ReportJob::Status::COMPLETED, ReportJob::Status::CANCELLED}) {
      auto value = job;
      value.m_id = std::to_string(static_cast<int>(status));
      value.m_status = status;
      unchanged.push_back(value);
      local.store(value);
    }
    auto sessions = WebSessionStore<WebPortalSession>();
    auto session = sessions.create();
    auto servlet = ReportingWebServlet(Ref(sessions), &local);
    REQUIRE(retry(servlet, *session, R"({"ids":["mine"]})").get_status_code() ==
      HttpStatusCode::UNAUTHORIZED);
    session->set_account(client.get_account());
    for(auto& body : {"", "{}", R"({"ids":1})", R"({"ids":[1]})"}) {
      REQUIRE(retry(servlet, *session, body).get_status_code() ==
        HttpStatusCode::BAD_REQUEST);
    }
    for(auto& body : {R"({"ids":["mine","other"]})",
        R"({"ids":["mine","missing"]})"}) {
      REQUIRE(retry(servlet, *session, body).get_status_code() ==
        HttpStatusCode::NOT_FOUND);
      REQUIRE(local.load_job("mine")->m_status == ReportJob::Status::FAILED);
    }
    REQUIRE(calls == 0);
    auto response =
      retry(servlet, *session, R"({"ids":["mine","mine","0","1","2","4"]})");
    REQUIRE(response.get_status_code() == HttpStatusCode::OK);
    REQUIRE(response.get_body().get_size() == 0);
    auto execution = executions.pop();
    REQUIRE(execution.m_id == job.m_id);
    REQUIRE(execution.m_definition.m_command == job.m_definition.m_command);
    REQUIRE(execution.m_arguments == job.m_arguments);
    REQUIRE(execution.m_error.empty());
    REQUIRE(!execution.m_exit_code);
    for(auto& value : unchanged) {
      REQUIRE(local.load_job(value.m_id)->m_status == value.m_status);
    }
    REQUIRE(retry(servlet, *session, R"({"ids":["mine"]})").get_status_code() ==
      HttpStatusCode::OK);
    REQUIRE(retry(servlet, *session, R"({"ids":[]})").get_status_code() ==
      HttpStatusCode::OK);
    local.cancel(client.get_account(), {job.m_id});
    local.close();
    REQUIRE(calls == 1);
    REQUIRE(retry(servlet, *session, R"({"ids":["mine"]})").get_status_code() ==
      HttpStatusCode::INTERNAL_SERVER_ERROR);
  }

  TEST_CASE("report_access") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto job = make_job(client.get_account());
    auto local = LocalReportService(
      {job.m_definition}, client, [] (const auto&, auto) { return 0; });
    local.store(job);
    auto restricted = job;
    restricted.m_id = "restricted";
    restricted.m_definition.m_id = "restricted_type";
    local.store(restricted);
    SUBCASE("removed") {
      local.set_definitions({job.m_definition});
    }
    SUBCASE("revoked") {
      auto definition = restricted.m_definition;
      definition.m_access = {};
      local.set_definitions({job.m_definition, definition});
    }
    REQUIRE_THROWS_AS(local.retry(job.m_account, {job.m_id, restricted.m_id}),
      ReportNotFoundException);
    REQUIRE(local.load_job(job.m_id)->m_status == ReportJob::Status::FAILED);
    REQUIRE(
      local.load_job(restricted.m_id)->m_status == ReportJob::Status::FAILED);
  }

  TEST_CASE("directory_access") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto directory = DirectoryEntry::make_directory(0);
    auto alice = client.make_account("alice", "", directory);
    auto bob = client.make_account("bob", "", directory);
    client.store(alice, bob, Permission::READ);
    auto job = make_job(alice);
    auto value = parse<JsonValue>(to_json(bob));
    SUBCASE("parameter") {
      job.m_definition.m_parameters =
        {{"account", "Account", "DirectoryEntry", true}};
      job.m_parameters["account"] = value;
    }
    SUBCASE("parameter_list") {
      job.m_definition.m_parameters =
        {{"accounts", "Accounts", "DirectoryEntryList", true}};
      job.m_parameters["accounts"] = std::vector{value};
    }
    SUBCASE("recipient") {
      job.m_recipients = {bob};
    }
    auto local = LocalReportService(
      {job.m_definition}, client, [] (const auto&, auto) { return 0; });
    local.store(job);
    REQUIRE_NOTHROW(validate_report_retries({job}, {job.m_definition}, client));
    client.store(alice, bob, Permissions());
    REQUIRE_THROWS_AS(local.retry(alice, {job.m_id}), std::invalid_argument);
    REQUIRE(local.load_job(job.m_id)->m_status == ReportJob::Status::FAILED);
  }
}
