#ifndef NEXUS_REPORT_SCHEDULE_BACKEND_HPP
#define NEXUS_REPORT_SCHEDULE_BACKEND_HPP
#include <Beam/Pointers/VirtualPtr.hpp>
#include "WebPortal/ReportJobService.hpp"

namespace Nexus {

  /** Provides the storage needed to dispatch scheduled reports. */
  template<typename T>
  concept IsReportScheduleBackend = IsReportJobBackend<T> &&
    requires(T& backend) {
      { backend.load_jobs() } -> std::same_as<std::vector<ReportJob>>;
      { backend.load_schedules() } -> std::same_as<std::vector<ReportSchedule>>;
      { backend.load_schedule(std::declval<const Beam::DirectoryEntry&>(),
          std::declval<const std::string&>()) } -> std::same_as<ReportSchedule>;
      { backend.load_definitions(
          std::declval<const Beam::DirectoryEntry&>()) } ->
            std::same_as<std::vector<ReportDefinition>>;
      { backend.store(std::declval<const ReportSchedule&>()) } ->
          std::same_as<void>;
      { backend.remove_schedule(std::declval<const std::string&>()) } ->
          std::same_as<void>;
    };

  /** Provides access to scheduled report storage and execution. */
  class ReportScheduleBackend {
    public:

      /**
       * Constructs a backend implementation in place.
       * @tparam T The backend implementation to construct.
       * @tparam Args The constructor argument types.
       * @param args The arguments forwarded to the backend constructor.
       */
      template<IsReportScheduleBackend T, typename... Args>
      explicit ReportScheduleBackend(std::in_place_type_t<T>, Args&&... args);

      /**
       * Wraps an existing backend implementation.
       * @tparam T The concrete backend or pointer type.
       * @param backend The implementation to wrap.
       */
      template<Beam::DisableCopy<ReportScheduleBackend> T> requires
        IsReportScheduleBackend<Beam::dereference_t<T>>
      ReportScheduleBackend(T&& backend);

      ReportScheduleBackend(const ReportScheduleBackend&) = default;
      ReportScheduleBackend(ReportScheduleBackend&&) = default;

      /** Loads the definitions currently available to an account. */
      std::vector<ReportDefinition> load_definitions(
        const Beam::DirectoryEntry& account);

      /** Loads all stored report jobs across accounts. */
      std::vector<ReportJob> load_jobs();

      /** Loads a job without ownership checks, or nullopt if absent. */
      std::optional<ReportJob> load_job(const std::string& id);

      /** Stores a report job's current state. */
      void store(const ReportJob& job);

      /** Executes a prepared report job and returns its exit code. */
      int execute(const ReportJob& job, std::stop_token stop);

      /** Removes job metadata and output without checking account ownership. */
      void remove(const std::string& id);

      /** Loads all stored report schedules across accounts. */
      std::vector<ReportSchedule> load_schedules();

      /** Loads a saved schedule for its owner. */
      ReportSchedule load_schedule(
        const Beam::DirectoryEntry& account, const std::string& id);

      /** Stores a schedule's configuration and dispatch state. */
      void store(const ReportSchedule& schedule);

      /** Removes a stored schedule without checking account ownership. */
      void remove_schedule(const std::string& id);

    private:
      class VirtualBackend {
        public:
          virtual ~VirtualBackend() = default;

          virtual std::vector<ReportDefinition> load_definitions(
            const Beam::DirectoryEntry& account) = 0;
          virtual std::vector<ReportJob> load_jobs() = 0;
          virtual std::optional<ReportJob> load_job(const std::string& id) = 0;
          virtual void store(const ReportJob& job) = 0;
          virtual int execute(const ReportJob& job, std::stop_token stop) = 0;
          virtual void remove(const std::string& id) = 0;
          virtual std::vector<ReportSchedule> load_schedules() = 0;
          virtual ReportSchedule load_schedule(
            const Beam::DirectoryEntry& account, const std::string& id) = 0;
          virtual void store(const ReportSchedule& schedule) = 0;
          virtual void remove_schedule(const std::string& id) = 0;
      };
      template<typename B>
      class WrappedBackend final : public VirtualBackend {
        public:
          using Backend = B;
          Beam::local_ptr_t<Backend> m_backend;

          template<typename... Args>
          WrappedBackend(Args&&... args);

          std::vector<ReportDefinition> load_definitions(
            const Beam::DirectoryEntry& account) override;
          std::vector<ReportJob> load_jobs() override;
          std::optional<ReportJob> load_job(const std::string& id) override;
          void store(const ReportJob& job) override;
          int execute(const ReportJob& job, std::stop_token stop) override;
          void remove(const std::string& id) override;
          std::vector<ReportSchedule> load_schedules() override;
          ReportSchedule load_schedule(const Beam::DirectoryEntry& account,
            const std::string& id) override;
          void store(const ReportSchedule& schedule) override;
          void remove_schedule(const std::string& id) override;
      };
      Beam::VirtualPtr<VirtualBackend> m_backend;
  };

  template<IsReportScheduleBackend T, typename... Args>
  ReportScheduleBackend::ReportScheduleBackend(
    std::in_place_type_t<T>, Args&&... args)
    : m_backend(Beam::make_virtual_ptr<WrappedBackend<T>>(
        std::forward<Args>(args)...)) {}

  template<Beam::DisableCopy<ReportScheduleBackend> T> requires
    IsReportScheduleBackend<Beam::dereference_t<T>>
  ReportScheduleBackend::ReportScheduleBackend(T&& backend)
    : m_backend(Beam::make_virtual_ptr<WrappedBackend<std::remove_cvref_t<T>>>(
        std::forward<T>(backend))) {}

  inline std::vector<ReportDefinition> ReportScheduleBackend::load_definitions(
      const Beam::DirectoryEntry& account) {
    return m_backend->load_definitions(account);
  }

  inline std::vector<ReportJob> ReportScheduleBackend::load_jobs() {
    return m_backend->load_jobs();
  }

  inline std::optional<ReportJob> ReportScheduleBackend::load_job(
      const std::string& id) {
    return m_backend->load_job(id);
  }

  inline void ReportScheduleBackend::store(const ReportJob& job) {
    m_backend->store(job);
  }

  inline int ReportScheduleBackend::execute(const ReportJob& job,
      std::stop_token stop) {
    return m_backend->execute(job, stop);
  }

  inline void ReportScheduleBackend::remove(const std::string& id) {
    m_backend->remove(id);
  }

  inline std::vector<ReportSchedule> ReportScheduleBackend::load_schedules() {
    return m_backend->load_schedules();
  }

  inline ReportSchedule ReportScheduleBackend::load_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_backend->load_schedule(account, id);
  }

  inline void ReportScheduleBackend::store(const ReportSchedule& schedule) {
    m_backend->store(schedule);
  }

  inline void ReportScheduleBackend::remove_schedule(const std::string& id) {
    m_backend->remove_schedule(id);
  }

  template<typename B>
  template<typename... Args>
  ReportScheduleBackend::WrappedBackend<B>::WrappedBackend(Args&&... args)
    : m_backend(std::forward<Args>(args)...) {}

  template<typename B>
  std::vector<ReportDefinition> ReportScheduleBackend::WrappedBackend<B>::
      load_definitions(const Beam::DirectoryEntry& account) {
    return m_backend->load_definitions(account);
  }

  template<typename B>
  std::vector<ReportJob> ReportScheduleBackend::WrappedBackend<B>::load_jobs() {
    return m_backend->load_jobs();
  }

  template<typename B>
  std::optional<ReportJob> ReportScheduleBackend::WrappedBackend<B>::load_job(
      const std::string& id) {
    return m_backend->load_job(id);
  }

  template<typename B>
  void ReportScheduleBackend::WrappedBackend<B>::store(const ReportJob& job) {
    m_backend->store(job);
  }

  template<typename B>
  int ReportScheduleBackend::WrappedBackend<B>::execute(const ReportJob& job,
      std::stop_token stop) {
    return m_backend->execute(job, stop);
  }

  template<typename B>
  void ReportScheduleBackend::WrappedBackend<B>::remove(const std::string& id) {
    m_backend->remove(id);
  }

  template<typename B>
  std::vector<ReportSchedule>
      ReportScheduleBackend::WrappedBackend<B>::load_schedules() {
    return m_backend->load_schedules();
  }

  template<typename B>
  ReportSchedule ReportScheduleBackend::WrappedBackend<B>::load_schedule(
      const Beam::DirectoryEntry& account, const std::string& id) {
    return m_backend->load_schedule(account, id);
  }

  template<typename B>
  void ReportScheduleBackend::WrappedBackend<B>::store(
      const ReportSchedule& schedule) {
    m_backend->store(schedule);
  }

  template<typename B>
  void ReportScheduleBackend::WrappedBackend<B>::remove_schedule(
      const std::string& id) {
    m_backend->remove_schedule(id);
  }
}

#endif
