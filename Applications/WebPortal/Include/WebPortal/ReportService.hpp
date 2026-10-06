#ifndef NEXUS_REPORT_SERVICE_HPP
#define NEXUS_REPORT_SERVICE_HPP
#include <stop_token>
#include <Beam/IO/Connection.hpp>
#include "WebPortal/GeneratedReport.hpp"
#include "WebPortal/ReportActivity.hpp"
#include "WebPortal/ReportDetail.hpp"
#include "WebPortal/ReportFile.hpp"
#include "WebPortal/ReportSubmission.hpp"
#include "WebPortal/ScheduledReport.hpp"

namespace Nexus {

  /** Provides report definitions, storage, and execution. */
  template<typename T>
  concept IsReportService = Beam::IsConnection<T> && requires(T& service) {
    { service.load_definitions(std::declval<const Beam::DirectoryEntry&>()) } ->
        std::same_as<std::vector<ReportDefinition>>;
    { service.query(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const GeneratedReportQuery&>()) } ->
          std::same_as<GeneratedReports>;
    { service.load_report(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::string&>()) } -> std::same_as<ReportDetail>;
    { service.load_file(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::string&>()) } -> std::same_as<ReportFile>;
    { service.query(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const ReportActivityQuery&>()) } ->
          std::same_as<ReportActivities>;
    { service.load_job(std::declval<const std::string&>()) } ->
        std::same_as<std::optional<ReportJob>>;
    { service.store(std::declval<const ReportJob&>()) } -> std::same_as<void>;
    { service.submit(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const ReportSubmission&>()) } -> std::same_as<std::string>;
    { service.share(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::vector<std::string>&>(),
        std::declval<const std::vector<Beam::DirectoryEntry>&>()) } ->
          std::same_as<void>;
    { service.cancel(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::vector<std::string>&>()) } ->
          std::same_as<void>;
    { service.retry(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::vector<std::string>&>()) } ->
          std::same_as<void>;
    { service.execute(std::declval<const ReportJob&>(), std::stop_token()) } ->
        std::same_as<int>;
    { service.remove(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::vector<std::string>&>()) } ->
          std::same_as<void>;
    { service.remove(std::declval<const std::string&>()) } ->
        std::same_as<void>;
    { service.query(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const ScheduledReportQuery&>()) } ->
          std::same_as<ScheduledReports>;
    { service.load_schedule(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::string&>()) } -> std::same_as<ReportSchedule>;
    { service.store(std::declval<const ReportSchedule&>()) } ->
        std::same_as<void>;
    { service.submit(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const ReportScheduleSubmission&>()) } ->
          std::same_as<std::string>;
    { service.update_schedule(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::string&>(),
        std::declval<const ReportScheduleSubmission&>()) } ->
          std::same_as<void>;
    { service.remove_schedule(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::string&>()) } -> std::same_as<void>;
  };

  /** Provides access to a report service. */
  class ReportService {
    public:

      /**
       * Constructs a service implementation in place.
       * @tparam T - The service implementation to construct.
       * @tparam Args - The constructor argument types.
       * @param args - The arguments forwarded to the service constructor.
       */
      template<IsReportService T, typename... Args>
      explicit ReportService(std::in_place_type_t<T>, Args&&... args);

      /**
       * Wraps an existing service implementation.
       * @tparam T - The concrete service or pointer type.
       * @param service - The implementation to wrap.
       */
      template<Beam::DisableCopy<ReportService> T> requires
        IsReportService<Beam::dereference_t<T>>
      ReportService(T&& service);

      ReportService(const ReportService&) = default;
      ReportService(ReportService&&) = default;

      /** Loads the definitions currently available to an account. */
      std::vector<ReportDefinition> load_definitions(
        const Beam::DirectoryEntry& account);

      /** Loads one page of completed reports accessible to an account. */
      GeneratedReports query(
        const Beam::DirectoryEntry& account, const GeneratedReportQuery& query);

      /**
       * Loads a completed report's saved metadata for an owner or recipient.
       * @throws ReportNotFoundException If the report is unavailable.
       */
      ReportDetail load_report(
        const Beam::DirectoryEntry& account, const std::string& id);

      /**
       * Loads a completed report's file for its owner or a sharing recipient.
       * @param account The account requesting the file.
       * @param id The report's job identifier.
       * @throws ReportNotFoundException If the report is unavailable.
       */
      ReportFile load_file(
        const Beam::DirectoryEntry& account, const std::string& id);

      /** Loads one page of pending and failed jobs submitted by an account. */
      ReportActivities query(const Beam::DirectoryEntry& account,
        const ReportActivityQuery& query);

      /** Loads stored job metadata, or nullopt if the job is absent. */
      std::optional<ReportJob> load_job(const std::string& id);

      /** Stores a job's current state. */
      void store(const ReportJob& job);

      /** Validates and queues a submission, returning its job identifier. */
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportSubmission& submission);

      /**
       * Adds recipients to completed reports owned by an account.
       * Existing recipients retain access. Global recipients are not allowed.
       * All reports and recipients are validated before any changes are stored.
       * @throws ReportNotFoundException If any report is missing, unfinished,
       *         or owned by another account.
       * @throws std::invalid_argument If a recipient is invalid or unreadable.
       */
      void share(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids,
        const std::vector<Beam::DirectoryEntry>& recipients);

      /**
       * Cancels jobs submitted by an account, removing them from activity.
       * Completed and already cancelled jobs are unchanged.
       * @param account The submitting account.
       * @param ids The job identifiers to cancel.
       * @throws ReportNotFoundException If any job is absent or owned by
       *         another account. No jobs are changed when this check fails.
       */
      void cancel(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);

      /**
       * Requeues failed jobs submitted by an account under their existing ids.
       * @param account The submitting account.
       * @param ids The job identifiers to retry.
       * @throws ReportNotFoundException If any job is absent, owned by another
       *         account, or a failed job's report type is no longer accessible.
       */
      void retry(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);

      /** Executes a stored job and returns its exit code. */
      int execute(const ReportJob& job, std::stop_token stop);

      /**
       * Deletes completed reports owned by an account and their output files.
       * All reports are checked for ownership and completion before deletion.
       * @throws ReportNotFoundException If a report is missing, unfinished,
       *         or owned by another account.
       */
      void remove(const Beam::DirectoryEntry& account,
        const std::vector<std::string>& ids);

      /** Removes stored job metadata and output. */
      void remove(const std::string& id);

      /** Loads one page of schedules owned by an account, newest first. */
      ScheduledReports query(
        const Beam::DirectoryEntry& account, const ScheduledReportQuery& query);

      /**
       * Loads a saved schedule for its owner.
       * @throws ReportNotFoundException If the schedule is missing or owned
       *         by another account.
       */
      ReportSchedule load_schedule(
        const Beam::DirectoryEntry& account, const std::string& id);

      /** Stores a schedule's configuration and next run time. */
      void store(const ReportSchedule& schedule);

      /** Validates and saves a schedule, returning its identifier. */
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportScheduleSubmission& submission);

      /**
       * Updates a schedule after checking ownership and permissions.
       * Preserves its identity, creation time, and unchanged timing.
       * @throws ReportNotFoundException If the schedule or report type is
       *         unavailable to the account.
       * @throws std::invalid_argument If the settings are invalid.
       */
      void update_schedule(const Beam::DirectoryEntry& account,
        const std::string& id, const ReportScheduleSubmission& submission);

      /**
       * Removes a schedule owned by an account, preserving existing reports.
       * @throws ReportNotFoundException If the schedule is missing or owned
       *         by another account.
       */
      void remove_schedule(
        const Beam::DirectoryEntry& account, const std::string& id);

      /** Stops execution and closes the service. */
      void close();

    private:
      struct VirtualReportService {
        virtual ~VirtualReportService() = default;

        virtual std::vector<ReportDefinition> load_definitions(
          const Beam::DirectoryEntry& account) = 0;
        virtual GeneratedReports query(const Beam::DirectoryEntry& account,
          const GeneratedReportQuery& query) = 0;
        virtual ReportDetail load_report(
          const Beam::DirectoryEntry& account, const std::string& id) = 0;
        virtual ReportFile load_file(
          const Beam::DirectoryEntry& account, const std::string& id) = 0;
        virtual ReportActivities query(const Beam::DirectoryEntry& account,
          const ReportActivityQuery& query) = 0;
        virtual std::optional<ReportJob> load_job(const std::string& id) = 0;
        virtual void store(const ReportJob& job) = 0;
        virtual std::string submit(const Beam::DirectoryEntry& account,
          const ReportSubmission& submission) = 0;
        virtual void share(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids,
          const std::vector<Beam::DirectoryEntry>& recipients) = 0;
        virtual void cancel(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) = 0;
        virtual void retry(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) = 0;
        virtual int execute(const ReportJob& job, std::stop_token stop) = 0;
        virtual void remove(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) = 0;
        virtual void remove(const std::string& id) = 0;
        virtual ScheduledReports query(const Beam::DirectoryEntry& account,
          const ScheduledReportQuery& query) = 0;
        virtual ReportSchedule load_schedule(
          const Beam::DirectoryEntry& account, const std::string& id) = 0;
        virtual void store(const ReportSchedule& schedule) = 0;
        virtual std::string submit(const Beam::DirectoryEntry& account,
          const ReportScheduleSubmission& submission) = 0;
        virtual void update_schedule(
          const Beam::DirectoryEntry& account, const std::string& id,
          const ReportScheduleSubmission& submission) = 0;
        virtual void remove_schedule(
          const Beam::DirectoryEntry& account, const std::string& id) = 0;
        virtual void close() = 0;
      };
      template<typename S>
      struct WrappedReportService final : VirtualReportService {
        using Service = S;
        Beam::local_ptr_t<Service> m_service;

        template<typename... Args>
        WrappedReportService(Args&&... args);

        std::vector<ReportDefinition> load_definitions(
          const Beam::DirectoryEntry& account) override;
        GeneratedReports query(const Beam::DirectoryEntry& account,
          const GeneratedReportQuery& query) override;
        ReportDetail load_report(const Beam::DirectoryEntry& account,
          const std::string& id) override;
        ReportFile load_file(const Beam::DirectoryEntry& account,
          const std::string& id) override;
        ReportActivities query(const Beam::DirectoryEntry& account,
          const ReportActivityQuery& query) override;
        std::optional<ReportJob> load_job(const std::string& id) override;
        void store(const ReportJob& job) override;
        std::string submit(const Beam::DirectoryEntry& account,
          const ReportSubmission& submission) override;
        void share(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids,
          const std::vector<Beam::DirectoryEntry>& recipients) override;
        void cancel(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) override;
        void retry(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) override;
        int execute(const ReportJob& job, std::stop_token stop) override;
        void remove(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) override;
        void remove(const std::string& id) override;
        ScheduledReports query(const Beam::DirectoryEntry& account,
          const ScheduledReportQuery& query) override;
        ReportSchedule load_schedule(
          const Beam::DirectoryEntry& account, const std::string& id) override;
        void store(const ReportSchedule& schedule) override;
        std::string submit(const Beam::DirectoryEntry& account,
          const ReportScheduleSubmission& submission) override;
        void update_schedule(
          const Beam::DirectoryEntry& account, const std::string& id,
          const ReportScheduleSubmission& submission) override;
        void remove_schedule(
          const Beam::DirectoryEntry& account, const std::string& id) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualReportService> m_service;
  };

  /**
   * Duplicates an owned schedule using the rules for submitting a new schedule.
   * @param service The service that loads and submits the schedule.
   * @param account The account requesting the duplicate.
   * @param id The source schedule identifier.
   * @return The newly saved schedule.
   */
  ReportSchedule duplicate_schedule(IsReportService auto& service,
      const Beam::DirectoryEntry& account, const std::string& id) {
    auto source = service.load_schedule(account, id);
    auto submission = ReportScheduleSubmission(
      ReportSubmission(source.m_definition.m_id, std::move(source.m_parameters),
        std::move(source.m_recipients)), source.m_start_time,
      source.m_repeat_interval, source.m_time_zone);
    auto duplicate = service.submit(account, submission);
    return service.load_schedule(account, duplicate);
  }

  /**
   * Queues an immediate job using an owned schedule's report settings.
   * @param service The service that loads and submits the report.
   * @param account The account requesting execution.
   * @param id The schedule identifier.
   * @return The submitted job identifier.
   */
  std::string run_schedule(IsReportService auto& service,
      const Beam::DirectoryEntry& account, const std::string& id) {
    auto schedule = service.load_schedule(account, id);
    auto submission = ReportSubmission(schedule.m_definition.m_id,
      std::move(schedule.m_parameters), std::move(schedule.m_recipients));
    return service.submit(account, submission);
  }

  template<IsReportService T, typename... Args>
  ReportService::ReportService(std::in_place_type_t<T>, Args&&... args)
    : m_service(Beam::make_virtual_ptr<WrappedReportService<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<ReportService> T> requires
    IsReportService<Beam::dereference_t<T>>
  ReportService::ReportService(T&& service)
    : m_service(Beam::make_virtual_ptr<
        WrappedReportService<std::remove_cvref_t<T>>>(
          std::forward<T>(service))) {}

  inline std::vector<ReportDefinition> ReportService::load_definitions(
      const Beam::DirectoryEntry& account) {
    return m_service->load_definitions(account);
  }

  inline GeneratedReports ReportService::query(
      const Beam::DirectoryEntry& account, const GeneratedReportQuery& query) {
    return m_service->query(account, query);
  }

  inline ReportDetail ReportService::load_report(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_service->load_report(account, id);
  }

  inline ReportFile ReportService::load_file(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_service->load_file(account, id);
  }

  inline ReportActivities ReportService::query(
      const Beam::DirectoryEntry& account, const ReportActivityQuery& query) {
    return m_service->query(account, query);
  }

  inline std::optional<ReportJob> ReportService::load_job(
      const std::string& id) {
    return m_service->load_job(id);
  }

  inline void ReportService::store(const ReportJob& job) {
    m_service->store(job);
  }

  inline std::string ReportService::submit(const Beam::DirectoryEntry& account,
      const ReportSubmission& submission) {
    return m_service->submit(account, submission);
  }

  inline void ReportService::share(
      const Beam::DirectoryEntry& account, const std::vector<std::string>& ids,
      const std::vector<Beam::DirectoryEntry>& recipients) {
    m_service->share(account, ids, recipients);
  }

  inline void ReportService::cancel(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->cancel(account, ids);
  }

  inline void ReportService::retry(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->retry(account, ids);
  }

  inline int ReportService::execute(
      const ReportJob& job, std::stop_token stop) {
    return m_service->execute(job, stop);
  }

  inline void ReportService::remove(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->remove(account, ids);
  }

  inline void ReportService::remove(const std::string& id) {
    m_service->remove(id);
  }

  inline ScheduledReports ReportService::query(
      const Beam::DirectoryEntry& account, const ScheduledReportQuery& query) {
    return m_service->query(account, query);
  }

  inline ReportSchedule ReportService::load_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_service->load_schedule(account, id);
  }

  inline void ReportService::store(const ReportSchedule& schedule) {
    m_service->store(schedule);
  }

  inline std::string ReportService::submit(const Beam::DirectoryEntry& account,
      const ReportScheduleSubmission& submission) {
    return m_service->submit(account, submission);
  }

  inline void ReportService::update_schedule(
      const Beam::DirectoryEntry& account, const std::string& id,
      const ReportScheduleSubmission& submission) {
    m_service->update_schedule(account, id, submission);
  }

  inline void ReportService::remove_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    m_service->remove_schedule(account, id);
  }

  inline void ReportService::close() {
    m_service->close();
  }

  template<typename S>
  template<typename... Args>
  ReportService::WrappedReportService<S>::WrappedReportService(Args&&... args)
    : m_service(std::forward<Args>(args)...) {}

  template<typename S>
  std::vector<ReportDefinition> ReportService::WrappedReportService<S>::
      load_definitions(const Beam::DirectoryEntry& account) {
    return m_service->load_definitions(account);
  }

  template<typename S>
  GeneratedReports ReportService::WrappedReportService<S>::query(
      const Beam::DirectoryEntry& account, const GeneratedReportQuery& query) {
    return m_service->query(account, query);
  }

  template<typename S>
  ReportDetail ReportService::WrappedReportService<S>::load_report(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_service->load_report(account, id);
  }

  template<typename S>
  ReportFile ReportService::WrappedReportService<S>::load_file(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_service->load_file(account, id);
  }

  template<typename S>
  ReportActivities ReportService::WrappedReportService<S>::query(
      const Beam::DirectoryEntry& account, const ReportActivityQuery& query) {
    return m_service->query(account, query);
  }

  template<typename S>
  std::optional<ReportJob> ReportService::WrappedReportService<S>::load_job(
      const std::string& id) {
    return m_service->load_job(id);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::store(const ReportJob& job) {
    m_service->store(job);
  }

  template<typename S>
  std::string ReportService::WrappedReportService<S>::submit(
      const Beam::DirectoryEntry& account, const ReportSubmission& submission) {
    return m_service->submit(account, submission);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::share(
      const Beam::DirectoryEntry& account, const std::vector<std::string>& ids,
      const std::vector<Beam::DirectoryEntry>& recipients) {
    m_service->share(account, ids, recipients);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::cancel(
      const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->cancel(account, ids);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::retry(
      const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->retry(account, ids);
  }

  template<typename S>
  int ReportService::WrappedReportService<S>::execute(
      const ReportJob& job, std::stop_token stop) {
    return m_service->execute(job, stop);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::remove(
      const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->remove(account, ids);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::remove(const std::string& id) {
    m_service->remove(id);
  }

  template<typename S>
  ScheduledReports ReportService::WrappedReportService<S>::query(
      const Beam::DirectoryEntry& account, const ScheduledReportQuery& query) {
    return m_service->query(account, query);
  }

  template<typename S>
  ReportSchedule ReportService::WrappedReportService<S>::load_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_service->load_schedule(account, id);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::store(
      const ReportSchedule& schedule) {
    m_service->store(schedule);
  }

  template<typename S>
  std::string ReportService::WrappedReportService<S>::submit(
      const Beam::DirectoryEntry& account,
      const ReportScheduleSubmission& submission) {
    return m_service->submit(account, submission);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::update_schedule(
      const Beam::DirectoryEntry& account, const std::string& id,
      const ReportScheduleSubmission& submission) {
    m_service->update_schedule(account, id, submission);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::remove_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    m_service->remove_schedule(account, id);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::close() {
    m_service->close();
  }

}

#endif
