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
#include <Beam/Serialization/ShuttleClone.hpp>
#include <Beam/TimeService/LocalTimeClient.hpp>
#include "WebPortal/ReportAccess.hpp"
#include "WebPortal/ReportJobService.hpp"
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
       * Constructs a local report service using the local clock.
       * @param definitions - The initial report definitions.
       * @param client - The client for permission and membership lookups.
       * @param executor - Executes prepared jobs and observes shutdown.
       */
      LocalReportService(std::vector<ReportDefinition> definitions,
        Beam::ServiceLocatorClient client, Executor executor);

      /**
       * Constructs a local report service.
       * @param definitions - The initial report definitions.
       * @param client - The client for permission and membership lookups.
       * @param executor - Executes prepared jobs and observes shutdown.
       * @param time_client - The client supplying job timestamps.
       */
      LocalReportService(std::vector<ReportDefinition> definitions,
        Beam::ServiceLocatorClient client, Executor executor,
        Beam::TimeClient time_client);

      ~LocalReportService();

      /** Returns job snapshots in submission order. */
      std::vector<ReportJob> load_jobs() const;

      /** Replaces the definitions available to subsequent requests. */
      void set_definitions(const std::vector<ReportDefinition>& definitions);

      /** Stores a job's generated output for subsequent downloads. */
      void set_output(const std::string& id, const Beam::SharedBuffer& output);

      GeneratedReports load_reports(
        const Beam::DirectoryEntry& account, const GeneratedReportQuery& query);
      ReportFile load_file(
        const Beam::DirectoryEntry& account, const std::string& id);
      ReportActivities load_activities(const Beam::DirectoryEntry& account,
        const ReportActivityQuery& query);
      std::vector<ReportDefinition> load_definitions(
        const Beam::DirectoryEntry& account);
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportSubmission& submission);
      void cancel(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);
      void retry(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);
      std::optional<ReportJob> load_job(const std::string& id);
      void store(const ReportJob& job);
      int execute(const ReportJob& job, std::stop_token stop);
      void close();

    private:
      Executor m_executor;
      Beam::ServiceLocatorClient m_client;
      Beam::TimeClient m_time_client;
      mutable std::mutex m_mutex;
      std::vector<ReportDefinition> m_definitions;
      std::vector<ReportJob> m_jobs;
      std::unordered_map<std::string, Beam::SharedBuffer> m_outputs;
      Beam::OpenState m_open_state;
      std::unique_ptr<Details::ReportJobService<
        LocalReportService, Beam::TimeClient>> m_service;

      LocalReportService(const LocalReportService&) = delete;
      LocalReportService& operator =(const LocalReportService&) = delete;
      std::vector<ReportDefinition> load_definitions() const;
  };

  template<IsReportExecutor E>
  LocalReportService<E>::LocalReportService(
      std::vector<ReportDefinition> definitions,
      Beam::ServiceLocatorClient client, Executor executor)
    : LocalReportService(std::move(definitions), std::move(client),
        std::move(executor), Beam::TimeClient(
          std::in_place_type<Beam::LocalTimeClient>)) {}

  template<IsReportExecutor E>
  LocalReportService<E>::LocalReportService(
      std::vector<ReportDefinition> definitions,
      Beam::ServiceLocatorClient client, Executor executor,
      Beam::TimeClient time_client)
      : m_executor(std::move(executor)),
        m_client(std::move(client)),
        m_time_client(std::move(time_client)),
        m_definitions(Beam::shuttle_clone(definitions)) {
    try {
      m_service = std::make_unique<
        Details::ReportJobService<LocalReportService, Beam::TimeClient>>(
          Beam::Ref(*this), m_time_client);
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
  std::vector<ReportJob> LocalReportService<E>::load_jobs() const {
    auto lock = std::lock_guard(m_mutex);
    return Beam::shuttle_clone(m_jobs);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::set_definitions(
      const std::vector<ReportDefinition>& definitions) {
    m_open_state.ensure_open();
    auto snapshot = Beam::shuttle_clone(definitions);
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
  GeneratedReports LocalReportService<E>::load_reports(
      const Beam::DirectoryEntry& account, const GeneratedReportQuery& query) {
    m_open_state.ensure_open();
    return query_generated_reports(load_jobs(), account, query, m_client);
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
    auto lock = std::lock_guard(m_mutex);
    auto output = m_outputs.find(id);
    if(output == m_outputs.end()) {
      throw ReportNotFoundException();
    }
    return ReportFile(make_report_filename(*job),
      job->m_definition.m_output.m_media_type, output->second);
  }

  template<IsReportExecutor E>
  ReportActivities LocalReportService<E>::load_activities(
      const Beam::DirectoryEntry& account, const ReportActivityQuery& query) {
    m_open_state.ensure_open();
    return query_report_activities(load_jobs(), account, query);
  }

  template<IsReportExecutor E>
  std::vector<ReportDefinition> LocalReportService<E>::load_definitions(
      const Beam::DirectoryEntry& account) {
    m_open_state.ensure_open();
    return filter_report_definitions(load_definitions(), account, m_client);
  }

  template<IsReportExecutor E>
  std::string LocalReportService<E>::submit(const Beam::DirectoryEntry& account,
      const ReportSubmission& submission) {
    m_open_state.ensure_open();
    return m_service->submit(prepare_report_job(
      load_definitions(), account, submission, m_client));
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
    m_service->retry(account, ids, [&] (const auto& jobs) {
      validate_report_retries(jobs, load_definitions(account), m_client);
    });
  }

  template<IsReportExecutor E>
  std::optional<ReportJob> LocalReportService<E>::load_job(
      const std::string& id) {
    auto lock = std::lock_guard(m_mutex);
    auto job = std::ranges::find(m_jobs, id, &ReportJob::m_id);
    if(job == m_jobs.end()) {
      return std::nullopt;
    }
    return Beam::shuttle_clone(*job);
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
  int LocalReportService<E>::execute(
      const ReportJob& job, std::stop_token stop) {
    return std::invoke(m_executor, job, stop);
  }

  template<IsReportExecutor E>
  void LocalReportService<E>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_service->close();
    m_open_state.close();
  }

  template<IsReportExecutor E>
  std::vector<ReportDefinition>
      LocalReportService<E>::load_definitions() const {
    auto lock = std::lock_guard(m_mutex);
    return Beam::shuttle_clone(m_definitions);
  }
}

#endif
