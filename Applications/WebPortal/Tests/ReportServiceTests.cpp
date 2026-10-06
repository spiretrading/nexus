#include <semaphore>
#include <Beam/Json/JsonParser.hpp>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "WebPortal/FileReportService.hpp"
#include "WebPortal/LocalReportService.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

static_assert(IsReportService<FileReportService>);
static_assert(IsReportService<
  LocalReportService<int (*)(const ReportJob&, std::stop_token)>>);
static_assert(IsReportService<ReportService>);

namespace {
  template<typename E>
  concept AcceptsExecutor = requires { typename LocalReportService<E>; };

  struct LvalueExecutor {
    int operator ()(const ReportJob&, std::stop_token&) &;
  };

  struct RvalueExecutor {
    int operator ()(const ReportJob&, std::stop_token) &&;
  };

  static_assert(AcceptsExecutor<LvalueExecutor>);
  static_assert(!AcceptsExecutor<RvalueExecutor>);
  static_assert(AcceptsExecutor<short (*)(const ReportJob&, std::stop_token)>);
  static_assert(!AcceptsExecutor<void (*)(const ReportJob&, std::stop_token)>);
  static_assert(!AcceptsExecutor<int (*)(ReportJob&, std::stop_token)>);
  static_assert(!AcceptsExecutor<int (*)(const ReportJob&, std::stop_token&&)>);

  struct RvalueReportService {
    ReportSchedule load_schedule(const DirectoryEntry&, const std::string&);
    ScheduledReports query(const DirectoryEntry&, const ScheduledReportQuery&);
    void store(const ReportSchedule&);
    ReportDetail load_report(const DirectoryEntry&, const std::string&);
    ReportFile load_file(const DirectoryEntry&, const std::string&);
    GeneratedReports query(const DirectoryEntry&, const GeneratedReportQuery&);
    ReportActivities query(const DirectoryEntry&, const ReportActivityQuery&);
    std::vector<ReportDefinition> load_definitions(DirectoryEntry&&);
    std::string submit(DirectoryEntry&&, ReportSubmission&&);
    void share(const DirectoryEntry&, const std::vector<std::string>&,
      const std::vector<DirectoryEntry>&);
    void remove(const DirectoryEntry&, const std::vector<std::string>&);
    void remove(const std::string&);
    void cancel(const DirectoryEntry&, const std::vector<std::string>&);
    void retry(const DirectoryEntry&, const std::vector<std::string>&);
    std::optional<ReportJob> load_job(const std::string&);
    void store(const ReportJob& job);
    int execute(const ReportJob& job, std::stop_token stop);
    void close();
  };

  static_assert(!IsReportService<RvalueReportService>);

  ReportDefinition make_definition() {
    auto definition = ReportDefinition();
    definition.m_id = "example";
    definition.m_name = "Example";
    definition.m_access = {"*"};
    definition.m_parameters = {
      {"count", "Count", "Integer", true, JsonValue(0)},
      {"period", "Period", "DateRange", false, parse<JsonValue>(
        R"({"start":"2026-10-01","end":"2026-10-05"})")}};
    definition.m_arguments = {"{count}", "{period.start}"};
    return definition;
  }
}

TEST_SUITE("ReportService") {
  TEST_CASE("store_and_execute") {
    auto environment = ServiceLocatorTestEnvironment();
    auto executions = std::vector<ReportJob>();
    auto local = LocalReportService({}, environment.get_root(),
      [&] (const auto& job, auto stop) {
        if(stop.stop_requested()) {
          throw std::runtime_error("Execution cancelled.");
        }
        executions.push_back(job);
        return 7;
      });
    auto service = ReportService(&local);
    auto job =
      ReportJob("job-id", {}, {}, make_definition(), {}, {"5", "20261001"});
    service.store(job);
    auto stored = local.load_jobs();
    REQUIRE(stored.size() == 1);
    REQUIRE(stored[0].m_id == job.m_id);
    REQUIRE(stored[0].m_status == ReportJob::Status::QUEUED);
    job.m_status = ReportJob::Status::RUNNING;
    service.store(job);
    stored = local.load_jobs();
    REQUIRE(stored.size() == 1);
    REQUIRE(stored[0].m_status == ReportJob::Status::RUNNING);
    REQUIRE(service.execute(job, std::stop_token()) == 7);
    REQUIRE(executions.size() == 1);
    REQUIRE(executions[0].m_id == job.m_id);
    REQUIRE(executions[0].m_arguments == job.m_arguments);
    auto source = std::stop_source();
    source.request_stop();
    REQUIRE_THROWS_AS(
      service.execute(job, source.get_token()), std::runtime_error);
    REQUIRE(executions.size() == 1);
  }

  TEST_CASE("local_submission_snapshot") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto definition = make_definition();
    auto executions = Queue<ReportJob>();
    auto release = std::binary_semaphore(0);
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto local = LocalReportService({definition}, client,
      [&] (const auto& job, auto stop) {
        executions.push(job);
        auto cancel = std::stop_callback(stop, [&] { release.release(); });
        release.acquire();
        return 0;
      }, &time_client);
    auto service = ReportService(&local);
    get<JsonObject>(
      *definition.m_parameters[1].m_default).set("start", "2000-01-01");
    auto loaded = service.load_definitions(client.get_account());
    REQUIRE(get<JsonObject>(*loaded[0].m_parameters[1].m_default).at("start") ==
      "2026-10-01");
    get<JsonObject>(
      *loaded[0].m_parameters[1].m_default).set("start", "2001-01-01");
    auto submission = ReportSubmission();
    submission.m_report_type = "example";
    submission.m_parameters["count"] = 5;
    submission.m_recipients = {
      DirectoryEntry::make_account(client.get_account().m_id, "forged"),
      client.get_account()};
    auto id = service.submit(client.get_account(), submission);
    auto executed = executions.pop();
    REQUIRE(executed.m_id == id);
    REQUIRE(executed.m_created == time_client.get_time());
    REQUIRE(executed.m_account == client.get_account());
    REQUIRE(executed.m_recipients.size() == 1);
    REQUIRE(executed.m_recipients[0].m_name == client.get_account().m_name);
    REQUIRE(
      executed.m_arguments == std::vector<std::string>({"5", "20261001"}));
    submission.m_parameters["count"] = 9;
    auto jobs = local.load_jobs();
    REQUIRE(jobs.size() == 1);
    REQUIRE(jobs[0].m_status == ReportJob::Status::RUNNING);
    REQUIRE(jobs[0].m_parameters.at("count") == 5);
    jobs[0].m_parameters["count"] = 10;
    REQUIRE(local.load_jobs()[0].m_parameters.at("count") == 5);
    service.close();
    REQUIRE(local.load_jobs()[0].m_status == ReportJob::Status::FAILED);
    REQUIRE_THROWS_AS(
      service.load_definitions(client.get_account()), EndOfFileException);
    REQUIRE_THROWS_AS(
      service.submit(client.get_account(), submission), EndOfFileException);
  }

  TEST_CASE("submission_permissions_and_validation") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto definition = make_definition();
    auto executor = [] (const auto&, auto) { return 0; };
    auto local = std::make_shared<LocalReportService<decltype(executor)>>(
      std::vector({definition}), client, executor);
    auto service = ReportService(local);
    auto submission = ReportSubmission();
    submission.m_report_type = "example";
    submission.m_parameters["count"] = 0.5;
    REQUIRE_THROWS_AS(
      service.submit(client.get_account(), submission), std::invalid_argument);
    submission.m_parameters["count"] = 0;
    submission.m_recipients = {DirectoryEntry::STAR_DIRECTORY};
    REQUIRE_THROWS_AS(
      service.submit(client.get_account(), submission), std::invalid_argument);
    submission.m_recipients.clear();
    REQUIRE(service.load_definitions(client.get_account()).size() == 1);
    definition.m_access.clear();
    local->set_definitions({definition});
    REQUIRE_THROWS_AS(service.submit(client.get_account(), submission),
      ReportNotFoundException);
    REQUIRE(service.load_definitions(client.get_account()).empty());
    REQUIRE(local->load_jobs().empty());
  }

  TEST_CASE("move_only_executor") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto results = std::make_unique<Queue<int>>();
    auto observer = results.get();
    auto executor =
      [results = std::move(results)] (const ReportJob&, std::stop_token) {
        results->push(7);
        return 7;
      };
    auto service =
      ReportService(std::in_place_type<LocalReportService<decltype(executor)>>,
        std::vector({make_definition()}), client, std::move(executor));
    auto submission = ReportSubmission();
    submission.m_report_type = "example";
    REQUIRE(!service.submit(client.get_account(), submission).empty());
    REQUIRE(observer->pop() == 7);
    service.close();
  }
}
