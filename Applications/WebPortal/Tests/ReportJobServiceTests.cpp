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
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ReportJob> m_jobs;

    Backend(Store store, Executor executor)
      : m_store(std::move(store)),
        m_executor(std::move(executor)) {}

    std::optional<ReportJob> load_job(const std::string& id) {
      auto lock = std::lock_guard(m_mutex);
      auto job = m_jobs.find(id);
      if(job == m_jobs.end()) {
        return std::nullopt;
      }
      return shuttle_clone(job->second);
    }

    void store(const ReportJob& job) {
      auto lock = std::lock_guard(m_mutex);
      m_store(job);
      m_jobs[job.m_id] = shuttle_clone(job);
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
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
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
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    auto job = make_job();
    auto id = service.submit(job);
    REQUIRE(!id.empty());
    auto queued = states.pop();
    REQUIRE(queued.m_id == id);
    REQUIRE(queued.m_status == ReportJob::Status::QUEUED);
    REQUIRE(queued.m_created == time_from_string("2026-10-05 12:00:00"));
    REQUIRE(queued.m_modified == queued.m_created);
    REQUIRE(states.pop().m_status == ReportJob::Status::RUNNING);
    job.m_parameters["count"] = 9;
    time_client.set(time_from_string("2026-10-05 12:05:00"));
    release.release();
    auto completed = states.pop();
    REQUIRE(completed.m_status == ReportJob::Status::COMPLETED);
    REQUIRE(completed.m_parameters.at("count") == 0);
    REQUIRE(completed.m_created == queued.m_created);
    REQUIRE(completed.m_completed == time_from_string("2026-10-05 12:05:00"));
    REQUIRE(completed.m_modified == completed.m_completed);
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
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
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
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
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
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    service.submit(make_job());
    REQUIRE(states.pop().m_status == ReportJob::Status::QUEUED);
    REQUIRE(states.pop().m_status == ReportJob::Status::RUNNING);
    service.submit(make_job());
    REQUIRE(states.pop().m_status == ReportJob::Status::QUEUED);
    service.close();
    REQUIRE(states.pop().m_status == ReportJob::Status::FAILED);
    REQUIRE(states.pop().m_status == ReportJob::Status::FAILED);
  }

  TEST_CASE("cancel_running_and_queued") {
    auto started = Queue<std::string>();
    auto release = std::binary_semaphore(0);
    auto calls = std::atomic_int(0);
    auto backend = Backend([] (const auto&) {},
      [&] (const auto& job, auto stop) {
        ++calls;
        auto cancel = std::stop_callback(stop, [&] { release.release(); });
        started.push(job.m_id);
        release.acquire();
        return 0;
      });
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    auto job = make_job();
    auto first = service.submit(job);
    REQUIRE(started.pop() == first);
    ThreadPool::get().wait_until_idle();
    auto second = service.submit(job);
    time_client.set(time_from_string("2026-10-05 12:01:00"));
    service.cancel(job.m_account, {second, first, second});
    service.close();
    REQUIRE(calls == 1);
    for(auto& id : {first, second}) {
      auto cancelled = backend.load_job(id);
      REQUIRE(cancelled->m_status == ReportJob::Status::CANCELLED);
      REQUIRE(cancelled->m_completed == time_client.get_time());
      REQUIRE(cancelled->m_modified == cancelled->m_completed);
    }
  }

  TEST_CASE("cancel_preserves_unrelated_jobs") {
    auto started = Queue<std::string>();
    auto completed = Queue<std::string>();
    auto release = std::binary_semaphore(0);
    auto calls = 0;
    auto backend = Backend([&] (const auto& job) {
      if(job.m_status == ReportJob::Status::COMPLETED) {
        completed.push(job.m_id);
      }
    }, [&] (const auto& job, auto stop) {
      ++calls;
      if(calls == 1) {
        auto cancel = std::stop_callback(stop, [&] { release.release(); });
        started.push(job.m_id);
        release.acquire();
      }
      return 0;
    });
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    auto job = make_job();
    auto first = service.submit(job);
    REQUIRE(started.pop() == first);
    auto second = service.submit(job);
    service.cancel(job.m_account, {first});
    REQUIRE(completed.pop() == second);
    service.close();
    REQUIRE(backend.load_job(first)->m_status == ReportJob::Status::CANCELLED);
    REQUIRE(backend.load_job(second)->m_status == ReportJob::Status::COMPLETED);
  }

  TEST_CASE("cancel_ownership_and_terminal_states") {
    auto backend =
      Backend([] (const auto&) {}, [] (const auto&, auto) { return 0; });
    auto job = make_job();
    job.m_id = "failed";
    job.m_status = ReportJob::Status::FAILED;
    backend.store(job);
    auto other = job;
    other.m_id = "other";
    other.m_account = DirectoryEntry::make_account(2, "bob");
    other.m_recipients = {job.m_account};
    backend.store(other);
    auto completed = job;
    completed.m_id = "completed";
    completed.m_status = ReportJob::Status::COMPLETED;
    backend.store(completed);
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    for(auto& id : {"other", "missing"}) {
      REQUIRE_THROWS_AS(
        service.cancel(job.m_account, {"failed", id}), ReportNotFoundException);
      REQUIRE(
        backend.load_job("failed")->m_status == ReportJob::Status::FAILED);
    }
    service.cancel(job.m_account, {});
    service.cancel(job.m_account, {"failed", "completed"});
    auto cancelled = backend.load_job("failed");
    REQUIRE(cancelled->m_status == ReportJob::Status::CANCELLED);
    REQUIRE(cancelled->m_modified == time_client.get_time());
    REQUIRE(backend.load_job("other")->m_status == ReportJob::Status::FAILED);
    REQUIRE(
      backend.load_job("completed")->m_status == ReportJob::Status::COMPLETED);
    time_client.set(time_from_string("2026-10-05 12:01:00"));
    service.cancel(job.m_account, {"failed"});
    REQUIRE(backend.load_job("failed")->m_modified == cancelled->m_modified);
    service.close();
    REQUIRE_THROWS_AS(service.cancel(job.m_account, {}), std::runtime_error);
  }

  TEST_CASE("cancel_store_failure") {
    auto started = Queue<std::string>();
    auto release = std::binary_semaphore(0);
    auto is_failing = std::atomic_bool(true);
    auto is_stopped = std::atomic_bool(false);
    auto backend = Backend([&] (const auto& job) {
      if(is_failing && job.m_status == ReportJob::Status::CANCELLED) {
        throw std::runtime_error("Unable to store cancellation.");
      }
    }, [&] (const auto& job, auto stop) {
      auto cancel = std::stop_callback(stop, [&] {
        is_stopped = true;
        release.release();
      });
      started.push(job.m_id);
      release.acquire();
      return 0;
    });
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    auto job = make_job();
    auto id = service.submit(job);
    REQUIRE(started.pop() == id);
    REQUIRE_THROWS_AS(service.cancel(job.m_account, {id}), std::runtime_error);
    REQUIRE(!is_stopped);
    REQUIRE(backend.load_job(id)->m_status == ReportJob::Status::RUNNING);
    is_failing = false;
    service.cancel(job.m_account, {id});
    service.close();
    REQUIRE(is_stopped);
    REQUIRE(backend.load_job(id)->m_status == ReportJob::Status::CANCELLED);
  }

  TEST_CASE("cancel_partial_store_failure") {
    auto started = Queue<std::string>();
    auto completed = Queue<std::string>();
    auto release = std::binary_semaphore(0);
    auto second = std::string();
    auto calls = 0;
    auto backend = Backend([&] (const auto& job) {
      if(job.m_status == ReportJob::Status::CANCELLED && job.m_id == second) {
        throw std::runtime_error("Unable to store cancellation.");
      }
      if(job.m_status == ReportJob::Status::COMPLETED) {
        completed.push(job.m_id);
      }
    }, [&] (const auto& job, auto stop) {
      ++calls;
      if(calls == 1) {
        auto cancel = std::stop_callback(stop, [&] { release.release(); });
        started.push(job.m_id);
        release.acquire();
      }
      return 0;
    });
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    auto job = make_job();
    auto first = service.submit(job);
    REQUIRE(started.pop() == first);
    second = service.submit(job);
    REQUIRE_THROWS_AS(
      service.cancel(job.m_account, {first, second}), std::runtime_error);
    REQUIRE(completed.pop() == second);
    service.close();
    REQUIRE(backend.load_job(first)->m_status == ReportJob::Status::CANCELLED);
    REQUIRE(backend.load_job(second)->m_status == ReportJob::Status::COMPLETED);
  }

  TEST_CASE("retry_failed_job") {
    auto job = make_job();
    job.m_id = "retry";
    job.m_status = ReportJob::Status::FAILED;
    job.m_created = time_from_string("2026-10-05 12:00:00");
    job.m_completed = time_from_string("2026-10-05 12:01:00");
    job.m_modified = job.m_completed;
    job.m_exit_code = 7;
    job.m_error = "Previous failure.";
    job.m_recipients = {DirectoryEntry::make_account(2, "Bob")};
    auto started = Queue<ReportJob>();
    auto finished = Queue<ReportJob>();
    auto release = std::binary_semaphore(0);
    auto calls = 0;
    auto backend = Backend([&] (const auto& value) {
      if(value.m_id == job.m_id &&
          (value.m_status == ReportJob::Status::FAILED ||
            value.m_status == ReportJob::Status::COMPLETED)) {
        finished.push(value);
      }
    }, [&] (const auto& value, auto stop) {
      ++calls;
      started.push(value);
      if(calls == 1) {
        auto cancel = std::stop_callback(stop, [&] {
          release.release();
        });
        release.acquire();
      } else if(calls == 2) {
        return 9;
      }
      return 0;
    });
    backend.m_jobs.emplace(job.m_id, job);
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:02:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    auto blocker = service.submit(make_job());
    REQUIRE(started.pop().m_id == blocker);
    service.retry(job.m_account, {job.m_id, job.m_id}, [] (const auto&) {});
    auto queued = backend.load_job(job.m_id);
    REQUIRE(queued->m_status == ReportJob::Status::QUEUED);
    REQUIRE(queued->m_created == job.m_created);
    REQUIRE(queued->m_modified == time_client.get_time());
    REQUIRE(queued->m_completed.is_not_a_date_time());
    REQUIRE(!queued->m_exit_code);
    REQUIRE(queued->m_error.empty());
    REQUIRE(queued->m_parameters == job.m_parameters);
    REQUIRE(queued->m_recipients == job.m_recipients);
    service.retry(job.m_account, {job.m_id}, [] (const auto&) {});
    service.cancel(job.m_account, {blocker});
    auto execution = started.pop();
    REQUIRE(execution.m_id == job.m_id);
    REQUIRE(execution.m_definition.m_command == job.m_definition.m_command);
    REQUIRE(execution.m_arguments == job.m_arguments);
    auto failed = finished.pop();
    REQUIRE(failed.m_status == ReportJob::Status::FAILED);
    REQUIRE(failed.m_exit_code == 9);
    service.retry(job.m_account, {job.m_id}, [] (const auto&) {});
    REQUIRE(started.pop().m_id == job.m_id);
    REQUIRE(finished.pop().m_status == ReportJob::Status::COMPLETED);
    service.close();
    REQUIRE(calls == 3);
    REQUIRE(
      backend.load_job(job.m_id)->m_status == ReportJob::Status::COMPLETED);
    REQUIRE_THROWS_AS(service.retry(job.m_account, {}, [] (const auto&) {}),
      std::runtime_error);
  }

  TEST_CASE("retry_store_failure") {
    auto job = make_job();
    job.m_id = "retry";
    job.m_status = ReportJob::Status::FAILED;
    auto is_failing = std::atomic_bool(true);
    auto calls = 0;
    auto finished = Queue<ReportJob>();
    auto backend = Backend([&] (const auto& value) {
      if(is_failing && value.m_status == ReportJob::Status::QUEUED) {
        throw std::runtime_error("Unable to store retry.");
      }
      if(value.m_status == ReportJob::Status::COMPLETED) {
        finished.push(value);
      }
    }, [&] (const auto&, auto) {
      ++calls;
      return 0;
    });
    backend.m_jobs.emplace(job.m_id, job);
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    REQUIRE_THROWS_AS(
      service.retry(job.m_account, {job.m_id}, [] (const auto&) {}),
      std::runtime_error);
    REQUIRE(backend.load_job(job.m_id)->m_status == ReportJob::Status::FAILED);
    is_failing = false;
    service.retry(job.m_account, {job.m_id}, [] (const auto&) {});
    REQUIRE(finished.pop().m_id == job.m_id);
    service.close();
    REQUIRE(calls == 1);
  }

  TEST_CASE("retry_cancellation") {
    auto job = make_job();
    job.m_id = "retry";
    job.m_status = ReportJob::Status::FAILED;
    auto started = Queue<std::string>();
    auto release = std::binary_semaphore(0);
    auto backend = Backend([] (const auto&) {},
      [&] (const auto& value, auto stop) {
        auto cancel = std::stop_callback(stop, [&] { release.release(); });
        started.push(value.m_id);
        release.acquire();
        return 0;
      });
    backend.m_jobs.emplace(job.m_id, job);
    auto time_client = FixedTimeClient(time_from_string("2026-10-05 12:00:00"));
    auto service = ReportJobService(Ref(backend), &time_client);
    service.retry(job.m_account, {job.m_id}, [] (const auto&) {});
    REQUIRE(started.pop() == job.m_id);
    service.retry(job.m_account, {job.m_id}, [] (const auto&) {});
    service.cancel(job.m_account, {job.m_id});
    service.retry(job.m_account, {job.m_id}, [] (const auto&) {});
    service.close();
    REQUIRE(
      backend.load_job(job.m_id)->m_status == ReportJob::Status::CANCELLED);
  }

  TEST_CASE("job_serialization") {
    auto job = make_job();
    job.m_id = "job-id";
    job.m_recipients = {DirectoryEntry::make_directory(2, "Reporting")};
    job.m_created = time_from_string("2026-10-05 12:00:00");
    job.m_completed = time_from_string("2026-10-05 12:00:01");
    job.m_modified = job.m_completed;
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
      REQUIRE(received.m_modified == job.m_modified);
      REQUIRE(received.m_status == job.m_status);
      REQUIRE(received.m_exit_code == job.m_exit_code);
      REQUIRE(received.m_error == job.m_error);
    });
  }
}
