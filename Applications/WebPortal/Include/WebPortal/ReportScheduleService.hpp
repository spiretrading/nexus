#ifndef NEXUS_REPORT_SCHEDULE_SERVICE_HPP
#define NEXUS_REPORT_SCHEDULE_SERVICE_HPP
#include <tuple>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
#include "WebPortal/ReportScheduleBackend.hpp"

namespace Nexus {

  /**
   * Dispatches scheduled report occurrences and coordinates schedule changes.
   * @tparam B The backend providing report and schedule storage.
   */
  template<typename B>
  class ReportScheduleService {
    public:

      /** The backend providing report and schedule storage. */
      using Backend = B;

      /** The worker receiving scheduled report jobs. */
      using JobService = ReportJobService<Backend, Beam::TimeClient>;

      /**
       * Recovers missed occurrences and starts processing timer events.
       * @param backend The report and schedule storage.
       * @param jobs The worker receiving queued jobs.
       * @param client The client used for permission and directory checks.
       * @param time_client The clock supplying the current UTC time.
       * @param timer The timer controlling schedule checks.
       */
      ReportScheduleService(Beam::Ref<Backend> backend,
        Beam::Ref<JobService> jobs, Beam::ServiceLocatorClient client,
        Beam::TimeClient time_client, Beam::Timer timer) requires
        IsReportScheduleBackend<Backend>;

      ~ReportScheduleService();

      /** Validates and stores a new schedule, returning its identifier. */
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportScheduleSubmission& submission);

      /** Updates a schedule owned by an account. */
      void update(const Beam::DirectoryEntry& account, const std::string& id,
        const ReportScheduleSubmission& submission);

      /** Removes a schedule owned by an account. */
      void remove(const Beam::DirectoryEntry& account, const std::string& id);

      /** Stops dispatching scheduled reports. */
      void close();

    private:
      ReportScheduleBackend m_backend;
      JobService* m_jobs;
      Beam::ServiceLocatorClient m_client;
      Beam::TimeClient m_time_client;
      boost::posix_time::ptime m_recovery_time;
      Beam::Timer m_timer;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_ticks;
      mutable Beam::Mutex m_mutex;
      Beam::OpenState m_open_state;
      Beam::RoutineHandler m_routine;
      std::deque<std::string> m_ready;

      ReportScheduleService(const ReportScheduleService&) = delete;
      ReportScheduleService& operator =(const ReportScheduleService&) = delete;
      void poll();
      void poll(bool is_recovery);
      void dispatch(ReportSchedule schedule, boost::posix_time::ptime now,
        bool is_recovery);
      void flush();
      void run();
  };

  template<typename B>
  ReportScheduleService<B>::ReportScheduleService(Beam::Ref<Backend> backend,
      Beam::Ref<JobService> jobs, Beam::ServiceLocatorClient client,
      Beam::TimeClient time_client, Beam::Timer timer) requires
      IsReportScheduleBackend<Backend>
      : m_backend(backend.get()),
        m_jobs(jobs.get()),
        m_client(std::move(client)),
        m_time_client(std::move(time_client)),
        m_recovery_time(m_time_client.get_time()),
        m_timer(std::move(timer)),
        m_ticks(std::make_shared<Beam::Queue<Beam::Timer::Result>>()) {
    try {
      poll(true);
      auto pending = std::unordered_set<std::string>();
      for(auto& schedule : m_backend.load_schedules()) {
        if(schedule.m_pending_job_id) {
          pending.insert(*schedule.m_pending_job_id);
        }
      }
      auto jobs = m_backend.load_jobs();
      std::ranges::sort(jobs, [] (const auto& left, const auto& right) {
        auto first = left.m_modified.value_or(left.m_created);
        auto second = right.m_modified.value_or(right.m_created);
        return std::tie(first, left.m_id) < std::tie(second, right.m_id);
      });
      m_ready.clear();
      for(auto& job : jobs) {
        if(!pending.contains(job.m_id) &&
            (job.m_status == ReportJob::Status::QUEUED ||
              job.m_status == ReportJob::Status::STAGED)) {
          m_ready.push_back(job.m_id);
        }
      }
      flush();
      m_timer.get_publisher().monitor(m_ticks);
      m_timer.start();
      m_routine = Beam::spawn([&] { run(); });
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename B>
  ReportScheduleService<B>::~ReportScheduleService() {
    close();
  }

  template<typename B>
  std::string ReportScheduleService<B>::submit(
      const Beam::DirectoryEntry& account,
      const ReportScheduleSubmission& submission) {
    auto lock = std::lock_guard(m_mutex);
    m_open_state.ensure_open();
    auto schedule = prepare_report_schedule(m_backend.load_definitions(account),
      account, submission, m_client, m_time_client.get_time());
    m_backend.store(schedule);
    return schedule.m_id;
  }

  template<typename B>
  void ReportScheduleService<B>::update(const Beam::DirectoryEntry& account,
      const std::string& id, const ReportScheduleSubmission& submission) {
    auto lock = std::lock_guard(m_mutex);
    m_open_state.ensure_open();
    auto schedule = m_backend.load_schedule(account, id);
    if(schedule.m_pending_job_id) {
      dispatch(schedule, m_time_client.get_time(), false);
      flush();
      schedule = m_backend.load_schedule(account, id);
    }
    auto updated = prepare_report_schedule(schedule, submission,
      m_backend.load_definitions(account), m_client, m_time_client.get_time());
    m_backend.store(updated);
  }

  template<typename B>
  void ReportScheduleService<B>::remove(
      const Beam::DirectoryEntry& account, const std::string& id) {
    auto lock = std::lock_guard(m_mutex);
    m_open_state.ensure_open();
    auto schedule = m_backend.load_schedule(account, id);
    if(schedule.m_pending_job_id) {
      dispatch(schedule, m_time_client.get_time(), false);
      flush();
    }
    try {
      m_backend.remove_schedule(id);
    } catch(const ReportNotFoundException&) {
    }
  }

  template<typename B>
  void ReportScheduleService<B>::close() {
    {
      auto lock = std::lock_guard(m_mutex);
      if(m_open_state.set_closing()) {
        return;
      }
      m_timer.cancel();
      m_ticks->close();
    }
    m_routine.wait();
    m_open_state.close();
  }

  template<typename B>
  void ReportScheduleService<B>::poll() {
    {
      auto lock = std::lock_guard(m_mutex);
      if(!m_open_state.is_open()) {
        return;
      }
      flush();
    }
    poll(false);
    auto lock = std::lock_guard(m_mutex);
    if(m_open_state.is_open()) {
      flush();
    }
  }

  template<typename B>
  void ReportScheduleService<B>::poll(bool is_recovery) {
    auto schedules = Beam::park([&] {
      return m_backend.load_schedules();
    });
    for(auto& snapshot : schedules) {
      auto lock = std::lock_guard(m_mutex);
      if(!m_open_state.is_open()) {
        return;
      }
      try {
        Beam::park([&] {
          auto schedule =
            m_backend.load_schedule(snapshot.m_account, snapshot.m_id);
          dispatch(std::move(schedule), m_time_client.get_time(), is_recovery);
        });
      } catch(const ReportNotFoundException&) {
      } catch(const std::exception&) {
        std::cerr << "Failed to dispatch report schedule " << snapshot.m_id <<
          ".\n" << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
    }
  }

  template<typename B>
  void ReportScheduleService<B>::dispatch(ReportSchedule schedule,
      boost::posix_time::ptime now, bool is_recovery) {
    if(schedule.m_job_id) {
      auto job = m_backend.load_job(*schedule.m_job_id);
      if(!job || (job->m_status != ReportJob::Status::QUEUED &&
          job->m_status != ReportJob::Status::RUNNING &&
          job->m_status != ReportJob::Status::STAGED)) {
        m_backend.remove_schedule(schedule.m_id);
      }
      return;
    }
    while(true) {
      auto due =
        convert_report_time(schedule.m_run_time, schedule.m_time_zone, "UTC");
      if(due > now && !schedule.m_pending_job_id) {
        return;
      }
      static const auto GENERATOR =
        boost::uuids::name_generator_sha1(boost::uuids::uuid());
      auto is_pending = schedule.m_pending_job_id.has_value();
      auto id = [&] {
        if(schedule.m_pending_job_id) {
          return *schedule.m_pending_job_id;
        }
        return boost::uuids::to_string(GENERATOR("nexus.report.schedule/" +
          schedule.m_id + '/' + boost::posix_time::to_iso_string(due)));
      }();
      if(!schedule.m_pending_job_id) {
        schedule.m_pending_job_id = id;
        m_backend.store(schedule);
      }
      auto job = m_backend.load_job(id);
      if(!job) {
        job.emplace();
        try {
          auto submission = ReportSubmission(schedule.m_definition.m_id,
            schedule.m_parameters, schedule.m_recipients);
          *job =
            prepare_report_job(m_backend.load_definitions(schedule.m_account),
              schedule.m_account, submission, m_client);
        } catch(const std::exception& exception) {
          *job = ReportJob(id, schedule.m_account, schedule.m_recipients,
            schedule.m_definition, schedule.m_parameters, {}, now, now, now,
            ReportJob::Status::STAGED, {}, exception.what(), false);
        }
        job->m_status = ReportJob::Status::STAGED;
        job->m_id = id;
        job->m_created = now;
        job->m_modified = now;
        m_backend.store(*job);
      }
      if(schedule.m_repeat_interval) {
        auto after = std::max(m_recovery_time, due);
        schedule.m_run_time = next_report_run(schedule, after);
      } else {
        schedule.m_job_id = id;
      }
      schedule.m_pending_job_id.reset();
      m_backend.store(schedule);
      m_ready.push_back(job->m_id);
      if(schedule.m_job_id) {
        if(job->m_status != ReportJob::Status::QUEUED &&
            job->m_status != ReportJob::Status::RUNNING &&
            job->m_status != ReportJob::Status::STAGED) {
          m_backend.remove_schedule(schedule.m_id);
        }
        return;
      }
      if(is_recovery || is_pending) {
        return;
      }
    }
  }

  template<typename B>
  void ReportScheduleService<B>::flush() {
    while(!m_ready.empty()) {
      try {
        m_jobs->resume(m_ready.front());
        m_ready.pop_front();
      } catch(const std::exception&) {
        std::cerr << "Failed to queue scheduled report " << m_ready.front() <<
          ".\n" << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
        return;
      }
    }
  }

  template<typename B>
  void ReportScheduleService<B>::run() {
    while(true) {
      try {
        if(m_ticks->pop() == Beam::Timer::Result::CANCELED) {
          return;
        }
      } catch(const Beam::PipeBrokenException&) {
        return;
      }
      try {
        poll();
      } catch(const std::exception&) {
        std::cerr << "Failed to check report schedules.\n" <<
          BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
      auto lock = std::lock_guard(m_mutex);
      if(!m_open_state.is_open()) {
        return;
      }
      m_timer.start();
    }
  }
}

#endif
