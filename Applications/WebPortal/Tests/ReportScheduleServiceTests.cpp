#include <semaphore>
#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "WebPortal/ReportScheduleService.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  class ScheduleTimer {
    public:
      using Result = Timer::Result;

      void trigger() {
        m_starts.pop();
        m_timer.trigger();
        m_starts.pop();
        m_starts.push(true);
      }

      void start() {
        m_timer.start();
        m_starts.push(true);
      }

      void cancel() {
        m_timer.cancel();
      }

      void wait() {
        m_timer.wait();
      }

      const Publisher<Result>& get_publisher() const {
        return m_timer.get_publisher();
      }

    private:
      TriggerTimer m_timer;
      Queue<bool> m_starts;
  };

  template<typename E>
  class Backend {
    public:
      using Executor = E;
      std::vector<ReportDefinition> m_definitions;
      std::unordered_map<std::string, ReportSchedule> m_schedules;
      std::unordered_map<std::string, ReportJob> m_jobs;
      mutable std::mutex m_mutex;
      Queue<ReportJob> m_states;
      Executor m_executor;
      bool m_is_commit_failing;
      bool m_is_ready_failing;

      explicit Backend(Executor executor)
        : m_executor(std::move(executor)),
          m_is_commit_failing(false),
          m_is_ready_failing(false) {}

      std::vector<ReportDefinition> load_definitions(const DirectoryEntry&) {
        return m_definitions;
      }

      std::vector<ReportSchedule> load_schedules() {
        auto lock = std::lock_guard(m_mutex);
        auto result = std::vector<ReportSchedule>();
        for(auto& [id, schedule] : m_schedules) {
          result.push_back(schedule);
        }
        return result;
      }

      ReportSchedule load_schedule(
          const DirectoryEntry& account, const std::string& id) {
        auto lock = std::lock_guard(m_mutex);
        auto schedule = m_schedules.find(id);
        if(schedule == m_schedules.end() || schedule->second.m_account !=
            account) {
          throw ReportNotFoundException();
        }
        return schedule->second;
      }

      std::vector<ReportJob> load_jobs() {
        auto lock = std::lock_guard(m_mutex);
        auto result = std::vector<ReportJob>();
        for(auto& [id, job] : m_jobs) {
          result.push_back(job);
        }
        return result;
      }

      std::optional<ReportJob> load_job(const std::string& id) {
        auto lock = std::lock_guard(m_mutex);
        auto job = m_jobs.find(id);
        if(job == m_jobs.end()) {
          return std::nullopt;
        }
        if(m_is_ready_failing &&
            job->second.m_status == ReportJob::Status::STAGED) {
          m_is_ready_failing = false;
          throw std::runtime_error("Injected queue handoff failure.");
        }
        return job->second;
      }

      void store(const ReportJob& job) {
        auto lock = std::lock_guard(m_mutex);
        m_jobs[job.m_id] = job;
        m_states.push(job);
      }

      void store(const ReportSchedule& schedule) {
        auto lock = std::lock_guard(m_mutex);
        if(m_is_commit_failing && !schedule.m_pending_job_id) {
          throw std::runtime_error("Injected schedule commit failure.");
        }
        m_schedules[schedule.m_id] = schedule;
      }

      void remove(const std::string& id) {
        auto lock = std::lock_guard(m_mutex);
        m_jobs.erase(id);
      }

      void remove_schedule(const std::string& id) {
        auto lock = std::lock_guard(m_mutex);
        if(m_schedules.erase(id) == 0) {
          throw ReportNotFoundException();
        }
      }

      int execute(const ReportJob& job, std::stop_token stop) {
        return m_executor(job, stop);
      }
  };

  ReportSchedule make_schedule(const DirectoryEntry& account) {
    auto definition = ReportDefinition("example", "Example", {}, {"*"},
      {{"count", "Count", "Integer", true}}, "program", {"{count}"});
    auto parameters = JsonObject();
    parameters["count"] = 1;
    return ReportSchedule("schedule", account, definition, parameters, {},
      time_from_string("2026-09-01 12:00:00"),
      time_from_string("2026-10-01 09:00:00"),
      time_from_string("2026-10-01 09:00:00"),
      ReportSchedule::Interval(1, ReportSchedule::Interval::Unit::DAY), "UTC");
  }

  ReportJob wait_for_terminal(auto& backend) {
    while(true) {
      auto job = backend.m_states.pop();
      if(job.m_status == ReportJob::Status::COMPLETED ||
          job.m_status == ReportJob::Status::FAILED ||
          job.m_status == ReportJob::Status::CANCELLED) {
        return job;
      }
    }
  }
}

TEST_SUITE("ReportScheduleService") {
  TEST_CASE("type_erased_backend") {
    static_assert(IsReportScheduleBackend<ReportScheduleBackend>);
    auto account = DirectoryEntry::make_account(1);
    auto schedule = make_schedule(account);
    auto executor = [] (const auto& job, auto stop) {
      return job.m_arguments.size() + int(stop.stop_requested());
    };
    auto backend = Backend(executor);
    backend.m_definitions = {schedule.m_definition};
    auto erased = ReportScheduleBackend(&backend);
    REQUIRE(erased.load_definitions(account).size() == 1);
    erased.store(schedule);
    REQUIRE(erased.load_schedules().size() == 1);
    REQUIRE(erased.load_schedule(account, schedule.m_id).m_id == schedule.m_id);
    auto job = ReportJob("job", account, {}, schedule.m_definition,
      schedule.m_parameters, {"one", "two"});
    erased.store(job);
    REQUIRE(erased.load_jobs().size() == 1);
    auto saved = erased.load_job(job.m_id);
    REQUIRE(saved);
    REQUIRE(saved->m_arguments == job.m_arguments);
    auto stop = std::stop_source();
    stop.request_stop();
    REQUIRE(erased.execute(job, stop.get_token()) == 3);
    erased.remove(job.m_id);
    REQUIRE(!erased.load_job(job.m_id));
    erased.remove_schedule(schedule.m_id);
    REQUIRE(erased.load_schedules().empty());
    auto owned = ReportScheduleBackend(
      std::in_place_type<Backend<decltype(executor)>>, executor);
    owned.store(job);
    REQUIRE(owned.load_jobs().size() == 1);
    REQUIRE(backend.load_jobs().empty());
  }

  TEST_CASE("startup_catch_up_and_overlapping_occurrences") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    auto started = Queue<ReportJob>();
    auto gate = std::binary_semaphore(0);
    auto backend = Backend([&] (const auto& job, auto stop) {
      auto cancel = std::stop_callback(stop, [&] { gate.release(); });
      started.push(job);
      gate.acquire();
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 2);
    auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
      TimeClient(&time), &timer);
    auto first = started.pop();
    REQUIRE(backend.load_jobs().size() == 1);
    REQUIRE(backend.load_schedule(source.m_account, source.m_id).m_run_time ==
      time_from_string("2026-10-07 09:00:00"));
    time.set(time_from_string("2026-10-09 12:00:00"));
    timer.trigger();
    auto jobs = backend.load_jobs();
    REQUIRE(jobs.size() == 4);
    REQUIRE(std::ranges::count(jobs, ReportJob::Status::QUEUED,
      &ReportJob::m_status) == 3);
    REQUIRE(backend.load_schedule(source.m_account, source.m_id).m_run_time ==
      time_from_string("2026-10-10 09:00:00"));
    timer.trigger();
    REQUIRE(backend.load_jobs().size() == 4);
    scheduler.close();
    worker.close();
    REQUIRE(
      backend.load_job(first.m_id)->m_status == ReportJob::Status::FAILED);
  }

  TEST_CASE("one_time_completion_and_edits") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    source.m_repeat_interval.reset();
    auto started = Queue<ReportJob>();
    auto gate = std::counting_semaphore<2>(0);
    auto backend = Backend([&] (const auto& job, auto stop) {
      auto cancel = std::stop_callback(stop, [&] { gate.release(); });
      started.push(job);
      gate.acquire();
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1);
    auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
      TimeClient(&time), &timer);
    auto job = started.pop();
    auto stored = backend.load_schedule(source.m_account, source.m_id);
    REQUIRE(stored.m_job_id == job.m_id);
    timer.trigger();
    REQUIRE(backend.load_jobs().size() == 1);
    auto submission = ReportScheduleSubmission(ReportSubmission(
      source.m_definition.m_id, source.m_parameters, {}), source.m_start_time,
      {}, source.m_time_zone);
    submission.m_report.m_parameters["count"] = 2;
    scheduler.update(source.m_account, source.m_id, submission);
    REQUIRE(backend.load_schedule(source.m_account, source.m_id).m_job_id ==
      job.m_id);
    auto is_rescheduled = false;
    SUBCASE("completion") {}
    SUBCASE("rescheduled") {
      submission.m_start_time = time_from_string("2026-10-08 09:00:00");
      scheduler.update(source.m_account, source.m_id, submission);
      is_rescheduled = true;
    }
    gate.release();
    REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
    timer.trigger();
    if(is_rescheduled) {
      auto saved = backend.load_schedule(source.m_account, source.m_id);
      REQUIRE(saved.m_run_time == submission.m_start_time);
      REQUIRE(!saved.m_job_id);
    } else {
      REQUIRE(backend.load_schedules().empty());
    }
    REQUIRE(backend.load_job(job.m_id)->m_parameters.at("count") == 1);
    scheduler.close();
    worker.close();
  }

  TEST_CASE("validation_failure_and_timer_dispatch") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto invalid = make_schedule(client.get_account());
    invalid.m_repeat_interval.reset();
    auto valid = invalid;
    valid.m_id = "valid";
    valid.m_definition.m_id = "valid";
    valid.m_start_time = time_from_string("2026-10-07 09:00:00");
    valid.m_run_time = valid.m_start_time;
    auto started = Queue<ReportJob>();
    auto backend = Backend([&] (const auto& job, auto) {
      started.push(job);
      return 0;
    });
    backend.m_definitions = {valid.m_definition};
    backend.store(invalid);
    backend.store(valid);
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1);
    auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
      TimeClient(&time), &timer);
    auto failed = wait_for_terminal(backend);
    REQUIRE(failed.m_status == ReportJob::Status::FAILED);
    REQUIRE(!failed.m_error.empty());
    REQUIRE(failed.m_account == invalid.m_account);
    timer.trigger();
    REQUIRE(backend.load_schedules().size() == 1);
    time.set(time_from_string("2026-10-07 09:00:00"));
    timer.trigger();
    auto job = started.pop();
    REQUIRE(job.m_definition.m_id == "valid");
    REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
    timer.trigger();
    REQUIRE(backend.load_schedules().empty());
    scheduler.close();
    worker.close();
  }

  TEST_CASE("recover_incomplete_occurrence_commit") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    auto started = Queue<ReportJob>();
    auto backend = Backend([&] (const auto& job, auto) {
      started.push(job);
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    backend.m_is_commit_failing = true;
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    {
      auto timer = ScheduleTimer();
      auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1);
      auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
        TimeClient(&time), &timer);
      REQUIRE(backend.load_jobs().size() == 1);
      REQUIRE(backend.load_jobs()[0].m_status == ReportJob::Status::STAGED);
      REQUIRE(backend.load_schedule(source.m_account, source.m_id).
        m_pending_job_id);
      scheduler.close();
      worker.close();
      REQUIRE(!started.try_pop());
    }
    auto id = backend.load_jobs()[0].m_id;
    backend.m_is_commit_failing = false;
    {
      auto timer = ScheduleTimer();
      auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1);
      auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
        TimeClient(&time), &timer);
      REQUIRE(started.pop().m_id == id);
      REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
      worker.resume(id);
      timer.trigger();
      scheduler.close();
      worker.close();
      REQUIRE(!started.try_pop());
      REQUIRE(backend.load_jobs().size() == 1);
      auto saved = backend.load_schedule(source.m_account, source.m_id);
      REQUIRE(!saved.m_pending_job_id);
      REQUIRE(saved.m_run_time == time_from_string("2026-10-07 09:00:00"));
    }
  }
  TEST_CASE("retry_queue_handoff_and_recover_submission_order") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    auto started = Queue<ReportJob>();
    auto gate = std::counting_semaphore<2>(0);
    auto backend = Backend([&] (const auto& job, auto stop) {
      auto cancel = std::stop_callback(stop, [&] { gate.release(); });
      started.push(job);
      gate.acquire();
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    auto older = ReportJob("older", source.m_account, {}, source.m_definition,
      source.m_parameters, {"1"}, time_from_string("2026-09-30 12:00:00"));
    backend.store(older);
    backend.m_is_ready_failing = true;
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1);
    auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
      TimeClient(&time), &timer);
    REQUIRE(started.pop().m_id == older.m_id);
    timer.trigger();
    gate.release();
    auto resumed = started.pop();
    REQUIRE(resumed.m_id != older.m_id);
    REQUIRE(backend.load_jobs().size() == 2);
    scheduler.close();
    worker.close();
  }

  TEST_CASE("retry_prepares_validation_failure") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    source.m_repeat_interval.reset();
    auto started = Queue<ReportJob>();
    auto backend = Backend([&] (const auto& job, auto) {
      started.push(job);
      return 0;
    });
    backend.store(source);
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1);
    auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
      TimeClient(&time), &timer);
    auto failed = wait_for_terminal(backend);
    REQUIRE(!failed.m_is_prepared);
    auto validate = [&] (auto& jobs) {
      prepare_report_retries(jobs, backend.m_definitions, client);
    };
    REQUIRE_THROWS_AS(worker.retry(source.m_account, {failed.m_id}, validate),
      ReportNotFoundException);
    backend.m_definitions = {source.m_definition};
    worker.retry(source.m_account, {failed.m_id}, validate);
    auto retried = started.pop();
    REQUIRE(retried.m_id == failed.m_id);
    REQUIRE(retried.m_is_prepared);
    REQUIRE(retried.m_arguments == std::vector<std::string>({"1"}));
    REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
    timer.trigger();
    REQUIRE(backend.load_schedules().empty());
    scheduler.close();
    worker.close();
  }

  TEST_CASE("recover_committed_one_time_job") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    source.m_repeat_interval.reset();
    source.m_job_id = "queued";
    auto started = Queue<ReportJob>();
    auto backend = Backend([&] (const auto& job, auto) {
      started.push(job);
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    auto job = ReportJob("queued", source.m_account, {}, source.m_definition,
      source.m_parameters, {"1"}, source.m_created);
    backend.store(job);
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1);
    auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
      TimeClient(&time), &timer);
    REQUIRE(started.pop().m_id == job.m_id);
    REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
    timer.trigger();
    REQUIRE(backend.load_schedules().empty());
    REQUIRE(backend.load_jobs().size() == 1);
    scheduler.close();
    worker.close();
  }
}
