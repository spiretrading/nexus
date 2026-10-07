#include <latch>
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
      Queue<ReportJob> m_failed_results;
      Executor m_executor;
      bool m_is_pending_failing;
      bool m_is_commit_failing;
      bool m_is_ready_failing;
      bool m_is_result_failing;

      explicit Backend(Executor executor)
        : m_executor(std::move(executor)),
          m_is_pending_failing(false),
          m_is_commit_failing(false),
          m_is_ready_failing(false),
          m_is_result_failing(false) {}

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
        if(m_is_result_failing &&
            (job.m_status == ReportJob::Status::COMPLETED ||
              job.m_status == ReportJob::Status::FAILED)) {
          m_is_result_failing = false;
          m_failed_results.push(job);
          throw std::runtime_error("Unable to store result.");
        }
        m_jobs[job.m_id] = job;
        m_states.push(job);
      }

      void store(const ReportSchedule& schedule) {
        auto lock = std::lock_guard(m_mutex);
        if(m_is_pending_failing && schedule.m_pending_job_id) {
          throw std::runtime_error("Injected pending marker failure.");
        }
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

  TEST_CASE("dispatch_with_one_available_pool_thread") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    source.m_start_time = time_from_string("2026-10-07 09:00:00");
    source.m_run_time = source.m_start_time;
    source.m_repeat_interval.reset();
    auto backend = Backend([] (const auto&, auto) { return 0; });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
    auto scheduler = ReportScheduleService(
      Ref(backend), Ref(worker), client, TimeClient(&time), &timer);
    auto count = std::max(1u, boost::thread::hardware_concurrency()) - 1;
    auto started = std::latch(count);
    auto release = std::counting_semaphore<>(0);
    auto blockers = RoutineHandlerGroup();
    for(auto i = 0u; i != count; ++i) {
      blockers.spawn([&] {
        park([&] {
          started.count_down();
          release.acquire();
        });
      });
    }
    started.wait();
    time.set(source.m_start_time);
    auto completed = std::binary_semaphore(0);
    auto dispatch = RoutineHandler(spawn([&] {
      timer.trigger();
      completed.release();
    }));
    auto unblock = boost::scope::scope_exit([&] {
      release.release(count);
    });
    constexpr auto TIMEOUT = std::chrono::seconds(10);
    REQUIRE(completed.try_acquire_for(TIMEOUT));
    REQUIRE(
      wait_for_terminal(backend).m_status == ReportJob::Status::COMPLETED);
    timer.trigger();
    REQUIRE(backend.load_schedules().empty());
    scheduler.close();
    worker.close();
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
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 2,
      Timer(std::in_place_type<TriggerTimer>));
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
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
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

  TEST_CASE("exhausted_recurrence") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    source.m_start_time = time_from_string("2026-10-07 09:00:00");
    source.m_repeat_interval =
      ReportSchedule::Interval(10000, ReportSchedule::Interval::Unit::YEAR);
    auto started = Queue<ReportJob>();
    auto gate = std::binary_semaphore(0);
    auto backend = Backend([&] (const auto& job, auto stop) {
      auto cancel = std::stop_callback(stop, [&] {
        gate.release();
      });
      started.push(job);
      gate.acquire();
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
    auto scheduler = ReportScheduleService(
      Ref(backend), Ref(worker), client, TimeClient(&time), &timer);
    auto submission = ReportScheduleSubmission(ReportSubmission(
      source.m_definition.m_id, source.m_parameters, {}), source.m_start_time,
      source.m_repeat_interval, source.m_time_zone);
    auto id = scheduler.submit(source.m_account, submission);
    auto is_pending = false;
    auto is_rescheduled = false;
    auto is_removed = false;
    SUBCASE("completion") {}
    SUBCASE("edit") {
      is_rescheduled = true;
    }
    SUBCASE("delete") {
      is_removed = true;
    }
    SUBCASE("pending_completion") {
      is_pending = true;
    }
    SUBCASE("pending_edit") {
      is_pending = true;
      is_rescheduled = true;
    }
    SUBCASE("pending_delete") {
      is_pending = true;
      is_removed = true;
    }
    backend.m_is_commit_failing = is_pending;
    time.set(source.m_start_time);
    timer.trigger();
    if(is_pending) {
      REQUIRE(backend.load_schedule(source.m_account, id).m_pending_job_id);
      REQUIRE(backend.load_jobs().size() == 1);
      REQUIRE(backend.load_jobs()[0].m_status == ReportJob::Status::STAGED);
      backend.m_is_commit_failing = false;
    }
    if(is_rescheduled) {
      submission.m_repeat_interval->m_count = 1;
      submission.m_repeat_interval->m_unit =
        ReportSchedule::Interval::Unit::DAY;
      scheduler.update(source.m_account, id, submission);
    } else if(is_removed) {
      scheduler.remove(source.m_account, id);
      REQUIRE(backend.load_schedules().empty());
    } else {
      timer.trigger();
      auto saved = backend.load_schedule(source.m_account, id);
      REQUIRE(!saved.m_pending_job_id);
      REQUIRE(saved.m_job_id);
    }
    timer.trigger();
    REQUIRE(backend.load_jobs().size() == 1);
    auto job = started.pop();
    REQUIRE(job.m_id == backend.load_jobs()[0].m_id);
    gate.release();
    REQUIRE(
      wait_for_terminal(backend).m_status == ReportJob::Status::COMPLETED);
    timer.trigger();
    if(is_rescheduled) {
      auto saved = backend.load_schedule(source.m_account, id);
      REQUIRE(!saved.m_job_id);
      REQUIRE(!saved.m_pending_job_id);
      REQUIRE(saved.m_run_time == time_from_string("2026-10-08 09:00:00"));
    } else {
      REQUIRE(backend.load_schedules().empty());
    }
    REQUIRE(backend.load_jobs().size() == 1);
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
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
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
    SUBCASE("recurring") {}
    SUBCASE("exhausted") {
      source.m_repeat_interval = ReportSchedule::Interval(
        10000, ReportSchedule::Interval::Unit::YEAR);
    }
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
      auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
        Timer(std::in_place_type<TriggerTimer>));
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
      auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
        Timer(std::in_place_type<TriggerTimer>));
      auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
        TimeClient(&time), &timer);
      REQUIRE(
        !backend.load_schedule(source.m_account, source.m_id).m_pending_job_id);
      REQUIRE(started.pop().m_id == id);
      REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
      worker.resume(id);
      timer.trigger();
      scheduler.close();
      worker.close();
      REQUIRE(!started.try_pop());
      REQUIRE(backend.load_jobs().size() == 1);
      if(source.m_repeat_interval->m_unit ==
          ReportSchedule::Interval::Unit::YEAR) {
        REQUIRE(backend.load_schedules().empty());
      } else {
        auto saved = backend.load_schedule(source.m_account, source.m_id);
        REQUIRE(!saved.m_pending_job_id);
        REQUIRE(saved.m_run_time == time_from_string("2026-10-07 09:00:00"));
      }
    }
  }
  TEST_CASE("retry_startup_catch_up") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    auto executions = std::atomic_int(0);
    auto backend = Backend([&] (const auto&, auto) {
      ++executions;
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    SUBCASE("pending_marker") {
      backend.m_is_pending_failing = true;
    }
    SUBCASE("schedule_commit") {
      backend.m_is_commit_failing = true;
    }
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
    auto scheduler = ReportScheduleService(
      Ref(backend), Ref(worker), client, TimeClient(&time), &timer);
    REQUIRE(executions == 0);
    backend.m_is_pending_failing = false;
    backend.m_is_commit_failing = false;
    timer.trigger();
    timer.trigger();
    REQUIRE(backend.load_jobs().size() == 1);
    REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
    REQUIRE(backend.load_schedule(source.m_account, source.m_id).m_run_time ==
      time_from_string("2026-10-07 09:00:00"));
    time.set(time_from_string("2026-10-09 12:00:00"));
    timer.trigger();
    REQUIRE(backend.load_jobs().size() == 4);
    for(auto i = 0; i != 3; ++i) {
      REQUIRE(
        wait_for_terminal(backend).m_status == ReportJob::Status::COMPLETED);
    }
    REQUIRE(executions == 4);
    scheduler.close();
    worker.close();
  }

  TEST_CASE("delayed_startup_catch_up") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    auto executions = std::atomic_int(0);
    auto backend = Backend([&] (const auto&, auto) {
      ++executions;
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.store(source);
    backend.m_is_commit_failing = true;
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
    auto scheduler = ReportScheduleService(
      Ref(backend), Ref(worker), client, TimeClient(&time), &timer);
    time.set(time_from_string("2026-10-09 12:00:00"));
    backend.m_is_commit_failing = false;
    timer.trigger();
    timer.trigger();
    REQUIRE(backend.load_jobs().size() == 4);
    for(auto i = 0; i != 4; ++i) {
      REQUIRE(
        wait_for_terminal(backend).m_status == ReportJob::Status::COMPLETED);
    }
    REQUIRE(executions == 4);
    REQUIRE(backend.load_schedule(source.m_account, source.m_id).m_run_time ==
      time_from_string("2026-10-10 09:00:00"));
    scheduler.close();
    worker.close();
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
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
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
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
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

  TEST_CASE("one_time_completion_store_retry") {
    auto environment = ServiceLocatorTestEnvironment();
    auto& client = environment.get_root();
    auto source = make_schedule(client.get_account());
    source.m_repeat_interval.reset();
    auto executions = std::atomic_int(0);
    auto backend = Backend([&] (const auto&, auto) {
      ++executions;
      return 0;
    });
    backend.m_definitions = {source.m_definition};
    backend.m_is_result_failing = true;
    backend.store(source);
    auto time = FixedTimeClient(time_from_string("2026-10-06 12:00:00"));
    auto timer = ScheduleTimer();
    auto retry_timer = TriggerTimer();
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      &retry_timer);
    auto scheduler = ReportScheduleService(Ref(backend), Ref(worker), client,
      TimeClient(&time), &timer);
    auto result = backend.m_failed_results.pop();
    timer.trigger();
    REQUIRE(backend.load_schedules().size() == 1);
    retry_timer.trigger();
    REQUIRE(wait_for_terminal(backend).m_status ==
      ReportJob::Status::COMPLETED);
    timer.trigger();
    REQUIRE(backend.load_schedules().empty());
    REQUIRE(backend.load_jobs().size() == 1);
    REQUIRE(backend.load_job(result.m_id)->m_status ==
      ReportJob::Status::COMPLETED);
    REQUIRE(executions == 1);
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
    auto worker = ReportJobService(Ref(backend), TimeClient(&time), 1,
      Timer(std::in_place_type<TriggerTimer>));
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
