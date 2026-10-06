#ifndef NEXUS_LOCAL_REPORT_SERVICE_HPP
#define NEXUS_LOCAL_REPORT_SERVICE_HPP
#include <algorithm>
#include <concepts>
#include <functional>
#include <mutex>
#include <stop_token>
#include <type_traits>
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include "WebPortal/ReportAccess.hpp"
#include "WebPortal/ReportScheduleService.hpp"
#include "WebPortal/ReportService.hpp"

namespace Nexus {

  /** A callable report executor. */
  template<typename T>
  concept IsReportExecutor =
    std::invocable<T&, const ReportJob&, std::stop_token&> &&
    std::convertible_to<
      std::invoke_result_t<T&, const ReportJob&, std::stop_token&>, int>;

  /**
   * Keeps report definitions and jobs in memory with controlled execution.
   * @tparam E - The callable used to execute prepared jobs.
   */
  template<IsReportExecutor E>
  class LocalReportService {
    public:

      /** The report executor. */
      using Executor = E;

      /**
       * Constructs a local report service with an execution limit.
       * @param definitions The initial report definitions.
       * @param client The client for permission and membership lookups.
       * @param executor Executes jobs, possibly concurrently across accounts.
       * @param time_client The client supplying job timestamps.
       * @param max_concurrency The positive global execution limit.
       * @param timer The timer controlling automatic schedule checks.
       * @param retry_timer The timer used to retry saving completed jobs.
       */
      LocalReportService(std::vector<ReportDefinition> definitions,
        Beam::ServiceLocatorClient client, Executor executor,
        Beam::TimeClient time_client, std::size_t max_concurrency,
        Beam::Timer timer, Beam::Timer retry_timer);

      ~LocalReportService();

      /** Replaces the definitions available to subsequent requests. */
      void set_definitions(const std::vector<ReportDefinition>& definitions);

      /** Stores a job's generated output for subsequent downloads. */
      void set_output(const std::string& id, const Beam::SharedBuffer& output);

      std::vector<ReportJob> load_jobs() const;
      std::vector<ReportSchedule> load_schedules() const;
      void remove_schedule(const std::string& id);
      std::vector<ReportDefinition> load_definitions(
        const Beam::DirectoryEntry& account);
      GeneratedReports query(
        const Beam::DirectoryEntry& account, const GeneratedReportQuery& query);
      ReportDetail load_report(
        const Beam::DirectoryEntry& account, const std::string& id);
      ReportFile load_file(
        const Beam::DirectoryEntry& account, const std::string& id);
      ReportActivities query(const Beam::DirectoryEntry& account,
        const ReportActivityQuery& query);
      std::optional<ReportJob> load_job(const std::string& id);
      void store(const ReportJob& job);
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportSubmission& submission);
      void share(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids,
        const std::vector<Beam::DirectoryEntry>& recipients);
      void cancel(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);
      void retry(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);
      int execute(const ReportJob& job, std::stop_token stop);
      void remove(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);
      void remove(const std::string& id);
      ScheduledReports query(
        const Beam::DirectoryEntry& account, const ScheduledReportQuery& query);
      ReportSchedule load_schedule(
        const Beam::DirectoryEntry& account, const std::string& id);
      void store(const ReportSchedule& schedule);
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportScheduleSubmission& submission);
      void update_schedule(const Beam::DirectoryEntry& account,
        const std::string& id, const ReportScheduleSubmission& submission);
      void remove_schedule(
        const Beam::DirectoryEntry& account, const std::string& id);
      void close();

    private:
      Executor m_executor;
      Beam::ServiceLocatorClient m_client;
      Beam::TimeClient m_time_client;
      mutable std::mutex m_mutex;
      std::vector<ReportDefinition> m_definitions;
      std::vector<ReportJob> m_jobs;
      std::vector<ReportSchedule> m_schedules;
      std::unordered_map<std::string, Beam::SharedBuffer> m_outputs;
      Beam::OpenState m_open_state;
      std::unique_ptr<ReportJobService<
        LocalReportService, Beam::TimeClient>> m_service;
      std::optional<ReportScheduleService<LocalReportService>> m_scheduler;

      LocalReportService(const LocalReportService&) = delete;
      LocalReportService& operator =(const LocalReportService&) = delete;
      std::vector<ReportDefinition> load_definitions() const;
  };

  template<IsReportExecutor E>
  LocalReportService<E>::LocalReportService(
      std::vector<ReportDefinition> definitions,
      Beam::ServiceLocatorClient client, Executor executor,
      Beam::TimeClient time_client, std::size_t max_concurrency,
      Beam::Timer timer, Beam::Timer retry_timer)
      : m_executor(std::move(executor)),
        m_client(std::move(client)),
        m_time_client(std::move(time_client)),
        m_definitions(std::move(definitions)) {
    try {
      m_service = std::make_unique<
        ReportJobService<LocalReportService, Beam::TimeClient>>(
          Beam::Ref(*this), m_time_client, max_concurrency,
          std::move(retry_timer));
      m_scheduler.emplace(Beam::Ref(*this), Beam::Ref(*m_service), m_client,
        m_time_client, std::move(timer));
    } catch(const std::exception&) {
      m_open_state.close();
      throw;
    }
  }

  template<IsReportExecutor E>
  LocalReportService<E>::~LocalReportService() {
    close();
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::set_definitions(
      const std::vector<ReportDefinition>& definitions) {
    m_open_state.ensure_open();
    auto snapshot = definitions;
    auto lock = std::lock_guard(m_mutex);
    m_definitions = std::move(snapshot);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::set_output(
      const std::string& id, const Beam::SharedBuffer& output) {
    m_open_state.ensure_open();
    auto lock = std::lock_guard(m_mutex);
    m_outputs[id] = output;
  }

  template<IsReportExecutor E>
  std::vector<ReportJob> LocalReportService<E>::load_jobs() const {
    auto lock = std::lock_guard(m_mutex);
    return m_jobs;
  }

  template<IsReportExecutor E>
  std::vector<ReportSchedule> LocalReportService<E>::load_schedules() const {
    auto lock = std::lock_guard(m_mutex);
    return m_schedules;
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::remove_schedule(const std::string& id) {
    m_open_state.ensure_open();
    auto count = [&] {
      auto lock = std::lock_guard(m_mutex);
      return std::erase_if(m_schedules, [&] (const auto& schedule) {
        return schedule.m_id == id;
      });
    }();
    if(count == 0) {
      throw ReportNotFoundException();
    }
  }

  template<IsReportExecutor E>
  std::vector<ReportDefinition> LocalReportService<E>::load_definitions(
      const Beam::DirectoryEntry& account) {
    m_open_state.ensure_open();
    return filter_report_definitions(load_definitions(), account, m_client);
  }

  template<IsReportExecutor E>
  GeneratedReports LocalReportService<E>::query(
      const Beam::DirectoryEntry& account, const GeneratedReportQuery& query) {
    m_open_state.ensure_open();
    return query_generated_reports(load_jobs(), account, query, m_client);
  }

  template<IsReportExecutor E>
  ReportDetail LocalReportService<E>::load_report(
      const Beam::DirectoryEntry& account, const std::string& id) {
    m_open_state.ensure_open();
    auto job = load_job(id);
    auto access = ReportAccess(account, m_client);
    if(!job || !access.is_accessible(*job)) {
      throw ReportNotFoundException();
    }
    return make_report_detail(*job);
  }

  template<IsReportExecutor E>
  ReportFile LocalReportService<E>::load_file(
      const Beam::DirectoryEntry& account, const std::string& id) {
    m_open_state.ensure_open();
    auto job = load_job(id);
    auto access = ReportAccess(account, m_client);
    if(!job || !access.is_accessible(*job)) {
      throw ReportNotFoundException();
    }
    auto output = [&] {
      auto lock = std::lock_guard(m_mutex);
      auto i = m_outputs.find(id);
      if(i == m_outputs.end()) {
        throw ReportNotFoundException();
      }
      return i->second;
    }();
    return ReportFile(make_report_filename(*job),
      job->m_definition.m_output.m_media_type, std::move(output));
  }

  template<IsReportExecutor E>
  ReportActivities LocalReportService<E>::query(
      const Beam::DirectoryEntry& account, const ReportActivityQuery& query) {
    m_open_state.ensure_open();
    return query_report_activities(load_jobs(), account, query);
  }

  template<IsReportExecutor E>
  std::optional<ReportJob> LocalReportService<E>::load_job(
      const std::string& id) {
    auto lock = std::lock_guard(m_mutex);
    auto job = std::ranges::find(m_jobs, id, &ReportJob::m_id);
    if(job == m_jobs.end()) {
      return std::nullopt;
    }
    return *job;
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::store(const ReportJob& job) {
    auto lock = std::lock_guard(m_mutex);
    auto existing = std::ranges::find(m_jobs, job.m_id, &ReportJob::m_id);
    if(existing == m_jobs.end()) {
      m_jobs.push_back(job);
    } else {
      *existing = job;
    }
  }

  template<IsReportExecutor E>
  std::string LocalReportService<E>::submit(const Beam::DirectoryEntry& account,
      const ReportSubmission& submission) {
    m_open_state.ensure_open();
    return m_service->submit(
      prepare_report_job(load_definitions(), account, submission, m_client));
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::share(
      const Beam::DirectoryEntry& account, const std::vector<std::string>& ids,
      const std::vector<Beam::DirectoryEntry>& recipients) {
    m_open_state.ensure_open();
    m_service->share(
      account, ids, prepare_report_recipients(recipients, account, m_client));
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::cancel(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_open_state.ensure_open();
    m_service->cancel(account, ids);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::retry(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_open_state.ensure_open();
    m_service->retry(account, ids, [&] (auto& jobs) {
      prepare_report_retries(jobs, load_definitions(account), m_client);
    });
  }

  template<IsReportExecutor E>
  int LocalReportService<E>::execute(
      const ReportJob& job, std::stop_token stop) {
    return std::invoke(m_executor, job, stop);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::remove(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_open_state.ensure_open();
    m_service->remove(account, ids);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::remove(const std::string& id) {
    auto lock = std::lock_guard(m_mutex);
    std::erase_if(m_jobs, [&] (const auto& job) {
      return job.m_id == id;
    });
    m_outputs.erase(id);
  }

  template<IsReportExecutor E>
  ScheduledReports LocalReportService<E>::query(
      const Beam::DirectoryEntry& account, const ScheduledReportQuery& query) {
    m_open_state.ensure_open();
    return query_scheduled_reports(load_schedules(), account, query);
  }

  template<IsReportExecutor E>
  ReportSchedule LocalReportService<E>::load_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    m_open_state.ensure_open();
    auto lock = std::lock_guard(m_mutex);
    auto schedule = std::ranges::find(m_schedules, id, &ReportSchedule::m_id);
    if(schedule == m_schedules.end() || schedule->m_account != account ||
        account.m_type != Beam::DirectoryEntry::Type::ACCOUNT) {
      throw ReportNotFoundException();
    }
    return *schedule;
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::store(const ReportSchedule& schedule) {
    m_open_state.ensure_open();
    auto snapshot = schedule;
    auto lock = std::lock_guard(m_mutex);
    auto existing =
      std::ranges::find(m_schedules, schedule.m_id, &ReportSchedule::m_id);
    if(existing == m_schedules.end()) {
      m_schedules.push_back(std::move(snapshot));
    } else {
      *existing = std::move(snapshot);
    }
  }

  template<IsReportExecutor E>
  std::string LocalReportService<E>::submit(const Beam::DirectoryEntry& account,
      const ReportScheduleSubmission& submission) {
    m_open_state.ensure_open();
    return m_scheduler->submit(account, submission);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::update_schedule(
      const Beam::DirectoryEntry& account, const std::string& id,
      const ReportScheduleSubmission& submission) {
    m_open_state.ensure_open();
    m_scheduler->update(account, id, submission);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::remove_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    m_open_state.ensure_open();
    m_scheduler->remove(account, id);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_scheduler->close();
    m_service->close();
    m_open_state.close();
  }

  template<IsReportExecutor E>
  std::vector<ReportDefinition>
      LocalReportService<E>::load_definitions() const {
    auto lock = std::lock_guard(m_mutex);
    return m_definitions;
  }
}

#endif
