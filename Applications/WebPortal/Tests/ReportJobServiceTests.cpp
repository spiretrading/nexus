#include <atomic>
#include <semaphore>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "WebPortal/ReportJobService.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  template<typename S, typename E>
  struct Backend {
    using Store = S;
    using Executor = E;
    Store m_store;
    Executor m_executor;

    Backend(Store store, Executor executor)
        : m_store(std::move(store)),
          m_executor(std::move(executor)) {}

    void store(const ReportJob& job) {
      m_store(job);
    }

    int execute(const ReportJob& job, std::stop_token stop) {
      return m_executor(job, stop);
    }
  };

  ReportJob make_job() {
    auto job = ReportJob();
    job.m_account = DirectoryEntry::make_account(1, "alice");
    job.m_definition.m_id = "example";
    job.m_definition.m_command = "report_program";
    job.m_parameters["count"] = 0;
    job.m_arguments = {"--count", "0"};
    return job;
  }
}

TEST_SUITE("ReportJobService") {
  TEST_CASE("routine_submission_and_shutdown") {
    auto states = Queue<ReportJob>();
    auto executions = Queue<ReportJob>();
    auto result = Queue<std::string>();
    auto release = std::binary_semaphore(0);
    auto backend = Backend([&] (const auto& job) {
      states.push(job);
    }, [&] (const auto& job, auto stop) {
      auto cancel = std::stop_callback(stop, [&] { release.release(); });
      executions.push(job);
      release.acquire();
      return 0;
    });
    auto time_client = FixedTimeClient(
      time_from_string("2026-10-05 12:00:00"));
    auto service = Nexus::Details::ReportJobService(
      Ref(backend), &time_client);
    auto routine = RoutineHandler(spawn([&] {
      try {
        auto id = service.submit(make_job());
        executions.pop();
        service.close();
        result.push(id);
      } catch(const std::exception&) {
        result.close(std::current_exception());
      }
    }));
    auto id = result.pop();
    routine.wait();
    REQUIRE(states.pop().m_id == id);
    REQUIRE(states.pop().m_status == ReportJob::Status::RUNNING);
    REQUIRE(states.pop().m_status == ReportJob::Status::FAILED);
  }

  TEST_CASE("submit_and_complete") {
    auto states = Queue<ReportJob>();
    auto release = std::counting_semaphore<2>(0);
    auto backend = Backend([&] (const auto& job) {
      states.push(job);
    }, [&] (const auto&, auto stop) {
      auto cancel = std::stop_callback(stop, [&] { release.release(); });
      release.acquire();
      return 0;
    });
    auto time_client = FixedTimeClient(
      time_from_string("2026-10-05 12:00:00"));
    auto service = Nexus::Details::ReportJobService(
      Ref(backend), &time_client);
    auto job = make_job();
    auto id = service.submit(job);
    REQUIRE(!id.empty());
    auto queued = states.pop();
    REQUIRE(queued.m_id == id);
    REQUIRE(queued.m_status == ReportJob::Status::QUEUED);
    REQUIRE(queued.m_created == time_from_string("2026-10-05 12:00:00"));
    REQUIRE(states.pop().m_status == ReportJob::Status::RUNNING);
    job.m_parameters["count"] = 9;
    time_client.set(time_from_string("2026-10-05 12:05:00"));
    release.release();
    auto completed = states.pop();
    REQUIRE(completed.m_status == ReportJob::Status::COMPLETED);
    REQUIRE(completed.m_parameters.at("count") == 0);
    REQUIRE(completed.m_created == queued.m_created);
    REQUIRE(completed.m_completed == time_from_string("2026-10-05 12:05:00"));
    REQUIRE(completed.m_exit_code == 0);
    service.close();
    REQUIRE_THROWS_AS(service.submit(job), std::runtime_error);
  }

  TEST_CASE("execution_failures") {
    auto states = Queue<ReportJob>();
    auto calls = 0;
    auto backend = Backend([&] (const auto& job) {
      states.push(job);
    }, [&] (const auto&, auto) {
      ++calls;
      if(calls == 1) {
        return 7;
      }
      throw std::runtime_error("Unable to launch process.");
    });
    auto time_client = FixedTimeClient(
      time_from_string("2026-10-05 12:00:00"));
    auto service = Nexus::Details::ReportJobService(
      Ref(backend), &time_client);
    auto first = service.submit(make_job());
    REQUIRE(states.pop().m_status == ReportJob::Status::QUEUED);
    REQUIRE(states.pop().m_status == ReportJob::Status::RUNNING);
    auto failed = states.pop();
    REQUIRE(failed.m_status == ReportJob::Status::FAILED);
    REQUIRE(failed.m_exit_code == 7);
    REQUIRE(failed.m_completed == time_client.get_time());
    REQUIRE(!failed.m_error.empty());
    auto second = service.submit(make_job());
    REQUIRE(first != second);
    REQUIRE(states.pop().m_status == ReportJob::Status::QUEUED);
    REQUIRE(states.pop().m_status == ReportJob::Status::RUNNING);
    failed = states.pop();
    REQUIRE(failed.m_status == ReportJob::Status::FAILED);
    REQUIRE(!failed.m_exit_code);
    REQUIRE(!failed.m_error.empty());
  }

  TEST_CASE("store_failure") {
    auto calls = std::atomic_int(0);
    auto backend = Backend([] (const auto&) {
      throw std::runtime_error("Unable to store job.");
    }, [&] (const auto&, auto) {
      ++calls;
      return 0;
    });
    auto time_client = FixedTimeClient(
      time_from_string("2026-10-05 12:00:00"));
    auto service = Nexus::Details::ReportJobService(
      Ref(backend), &time_client);
    REQUIRE_THROWS_AS(service.submit(make_job()), std::runtime_error);
    service.close();
    REQUIRE(calls == 0);
  }

  TEST_CASE("shutdown") {
    auto states = Queue<ReportJob>();
    auto release = std::binary_semaphore(0);
    auto backend = Backend([&] (const auto& job) {
      states.push(job);
    }, [&] (const auto&, auto stop) {
      auto cancel = std::stop_callback(stop, [&] { release.release(); });
      release.acquire();
      return 0;
    });
    auto time_client = FixedTimeClient(
      time_from_string("2026-10-05 12:00:00"));
    auto service = Nexus::Details::ReportJobService(
      Ref(backend), &time_client);
    service.submit(make_job());
    REQUIRE(states.pop().m_status == ReportJob::Status::QUEUED);
    REQUIRE(states.pop().m_status == ReportJob::Status::RUNNING);
    service.submit(make_job());
    REQUIRE(states.pop().m_status == ReportJob::Status::QUEUED);
    service.close();
    REQUIRE(states.pop().m_status == ReportJob::Status::FAILED);
    REQUIRE(states.pop().m_status == ReportJob::Status::FAILED);
  }

  TEST_CASE("job_serialization") {
    auto job = make_job();
    job.m_id = "job-id";
    job.m_recipients = {DirectoryEntry::make_directory(2, "Reporting")};
    job.m_created = time_from_string("2026-10-05 12:00:00");
    job.m_completed = time_from_string("2026-10-05 12:00:01");
    job.m_status = ReportJob::Status::FAILED;
    job.m_exit_code = 7;
    job.m_error = "Process failed.";
    test_round_trip_shuttle(job, [&] (const auto& received) {
      REQUIRE(received.m_id == job.m_id);
      REQUIRE(received.m_account == job.m_account);
      REQUIRE(received.m_recipients == job.m_recipients);
      REQUIRE(received.m_parameters == job.m_parameters);
      REQUIRE(received.m_arguments == job.m_arguments);
      REQUIRE(received.m_created == job.m_created);
      REQUIRE(received.m_completed == job.m_completed);
      REQUIRE(received.m_status == job.m_status);
      REQUIRE(received.m_exit_code == job.m_exit_code);
      REQUIRE(received.m_error == job.m_error);
    });
  }
}
