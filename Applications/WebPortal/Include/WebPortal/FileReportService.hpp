#ifndef NEXUS_FILE_REPORT_SERVICE_HPP
#define NEXUS_FILE_REPORT_SERVICE_HPP
#include <filesystem>
#include <mutex>
#include <stop_token>
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include "WebPortal/ReportScheduleService.hpp"
#include "WebPortal/ReportService.hpp"

namespace Nexus {

  /** Executes reports using YAML definitions and persistent job files. */
  class FileReportService {
    public:

      /**
       * Constructs a file report service.
       * @param definitions_directory - The directory of YAML definitions.
       * @param jobs_directory - The directory for job metadata and output.
       * @param client - The client for permission and membership lookups.
       * @param time_client - The client supplying job timestamps.
       * @param max_concurrency - The positive global execution limit.
       * @param timer The timer controlling automatic schedule checks.
       * @param retry_timer The timer used to retry saving completed jobs.
       */
      FileReportService(std::filesystem::path definitions_directory,
        std::filesystem::path jobs_directory,
        Beam::ServiceLocatorClient client, Beam::TimeClient time_client,
        std::size_t max_concurrency, Beam::Timer timer,
        Beam::Timer retry_timer);

      ~FileReportService();

      std::vector<ReportJob> load_jobs();
      std::vector<ReportSchedule> load_schedules();
      void remove_schedule(const std::string& id);
      std::vector<ReportDefinition> load_definitions(
        const Beam::DirectoryEntry& account);
      GeneratedReports query(
        const Beam::DirectoryEntry& account, const GeneratedReportQuery& query);
      ReportDetail load_report(
        const Beam::DirectoryEntry& account, const std::string& id);
      ReportFile load_file(
        const Beam::DirectoryEntry& account, const std::string& id);
      ReportActivities query(
        const Beam::DirectoryEntry& account, const ReportActivityQuery& query);
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
      std::filesystem::path m_definitions_directory;
      std::filesystem::path m_jobs_directory;
      Beam::ServiceLocatorClient m_client;
      Beam::TimeClient m_time_client;
      mutable std::mutex m_mutex;
      std::unordered_map<std::string, std::string> m_filenames;
      Beam::OpenState m_open_state;
      std::optional<ReportJobService<FileReportService, Beam::TimeClient>>
        m_jobs;
      std::optional<ReportScheduleService<FileReportService>> m_schedules;

      FileReportService(const FileReportService&) = delete;
      FileReportService& operator =(const FileReportService&) = delete;
      std::optional<std::string> read_metadata(
        const std::filesystem::path& directory);
      void remove_directory(
        const std::string& id, const std::filesystem::path& subdirectory);
      std::vector<ReportDefinition> load_definitions();
      std::optional<ReportJob> read_job(const std::filesystem::path& path);
      void recover();
      std::optional<ReportSchedule> read_schedule(
        const std::filesystem::path& path);
  };
}

#endif
