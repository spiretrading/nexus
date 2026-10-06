#ifndef NEXUS_REPORT_SERVICE_HPP
#define NEXUS_REPORT_SERVICE_HPP
#include <stop_token>
#include <Beam/IO/Connection.hpp>
#include "WebPortal/ReportActivity.hpp"
#include "WebPortal/ReportSubmission.hpp"

namespace Nexus {

  /** Provides report definitions, storage, and execution.
   * @tparam T - The concrete report service type.
   */
  template<typename T>
  concept IsReportService = Beam::IsConnection<T> && requires(T& service) {
    { service.cancel(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::vector<std::string>&>()) } ->
        std::same_as<void>;
    { service.retry(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const std::vector<std::string>&>()) } ->
        std::same_as<void>;
    { service.load_job(std::declval<const std::string&>()) } ->
        std::same_as<std::optional<ReportJob>>;
    { service.load_activities(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const ReportActivityQuery&>()) } ->
        std::same_as<ReportActivities>;
    { service.load_definitions(
        std::declval<const Beam::DirectoryEntry&>()) } ->
        std::same_as<std::vector<ReportDefinition>>;
    { service.submit(std::declval<const Beam::DirectoryEntry&>(),
        std::declval<const ReportSubmission&>()) } ->
        std::same_as<std::string>;
    { service.store(std::declval<const ReportJob&>()) } -> std::same_as<void>;
    { service.execute(std::declval<const ReportJob&>(), std::stop_token()) } ->
        std::same_as<int>;
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

      /** Loads one page of pending and failed jobs submitted by an account. */
      ReportActivities load_activities(const Beam::DirectoryEntry& account,
        const ReportActivityQuery& query);

      /** Loads the definitions currently available to an account. */
      std::vector<ReportDefinition> load_definitions(
        const Beam::DirectoryEntry& account);

      /** Validates and queues a submission, returning its job identifier. */
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportSubmission& submission);

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

      /** Loads stored job metadata, or nullopt if the job is absent. */
      std::optional<ReportJob> load_job(const std::string& id);

      /** Stores a job's current state. */
      void store(const ReportJob& job);

      /** Executes a stored job and returns its exit code. */
      int execute(const ReportJob& job, std::stop_token stop);

      /** Stops execution and closes the service. */
      void close();

    private:
      struct VirtualReportService {
        virtual ~VirtualReportService() = default;

        virtual ReportActivities load_activities(
          const Beam::DirectoryEntry& account,
          const ReportActivityQuery& query) = 0;
        virtual std::vector<ReportDefinition> load_definitions(
          const Beam::DirectoryEntry& account) = 0;
        virtual std::string submit(const Beam::DirectoryEntry& account,
          const ReportSubmission& submission) = 0;
        virtual void cancel(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) = 0;
        virtual void retry(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) = 0;
        virtual std::optional<ReportJob> load_job(const std::string& id) = 0;
        virtual void store(const ReportJob& job) = 0;
        virtual int execute(const ReportJob& job, std::stop_token stop) = 0;
        virtual void close() = 0;
      };
      template<typename S>
      struct WrappedReportService final : VirtualReportService {
        using Service = S;
        Beam::local_ptr_t<Service> m_service;

        template<typename... Args>
        WrappedReportService(Args&&... args);

        ReportActivities load_activities(const Beam::DirectoryEntry& account,
          const ReportActivityQuery& query) override;
        std::vector<ReportDefinition> load_definitions(
          const Beam::DirectoryEntry& account) override;
        std::string submit(const Beam::DirectoryEntry& account,
          const ReportSubmission& submission) override;
        void cancel(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) override;
        void retry(const Beam::DirectoryEntry& account,
          const std::vector<std::string>& ids) override;
        std::optional<ReportJob> load_job(const std::string& id) override;
        void store(const ReportJob& job) override;
        int execute(const ReportJob& job, std::stop_token stop) override;
        void close() override;
      };
      Beam::VirtualPtr<VirtualReportService> m_service;
  };

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

  inline ReportActivities ReportService::load_activities(
      const Beam::DirectoryEntry& account, const ReportActivityQuery& query) {
    return m_service->load_activities(account, query);
  }

  inline std::vector<ReportDefinition> ReportService::load_definitions(
      const Beam::DirectoryEntry& account) {
    return m_service->load_definitions(account);
  }

  inline std::string ReportService::submit(const Beam::DirectoryEntry& account,
      const ReportSubmission& submission) {
    return m_service->submit(account, submission);
  }

  inline void ReportService::cancel(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->cancel(account, ids);
  }

  inline void ReportService::retry(const Beam::DirectoryEntry& account,
      const std::vector<std::string>& ids) {
    m_service->retry(account, ids);
  }

  inline std::optional<ReportJob> ReportService::load_job(
      const std::string& id) {
    return m_service->load_job(id);
  }

  inline void ReportService::store(const ReportJob& job) {
    m_service->store(job);
  }

  inline int ReportService::execute(
      const ReportJob& job, std::stop_token stop) {
    return m_service->execute(job, stop);
  }

  inline void ReportService::close() {
    m_service->close();
  }

  template<typename S>
  template<typename... Args>
  ReportService::WrappedReportService<S>::WrappedReportService(Args&&... args)
    : m_service(std::forward<Args>(args)...) {}

  template<typename S>
  ReportActivities ReportService::WrappedReportService<S>::load_activities(
      const Beam::DirectoryEntry& account, const ReportActivityQuery& query) {
    return m_service->load_activities(account, query);
  }

  template<typename S>
  std::vector<ReportDefinition> ReportService::WrappedReportService<S>::
      load_definitions(const Beam::DirectoryEntry& account) {
    return m_service->load_definitions(account);
  }

  template<typename S>
  std::string ReportService::WrappedReportService<S>::submit(
      const Beam::DirectoryEntry& account, const ReportSubmission& submission) {
    return m_service->submit(account, submission);
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
  std::optional<ReportJob> ReportService::WrappedReportService<S>::load_job(
      const std::string& id) {
    return m_service->load_job(id);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::store(const ReportJob& job) {
    m_service->store(job);
  }

  template<typename S>
  int ReportService::WrappedReportService<S>::execute(
      const ReportJob& job, std::stop_token stop) {
    return m_service->execute(job, stop);
  }

  template<typename S>
  void ReportService::WrappedReportService<S>::close() {
    m_service->close();
  }
}

#endif
