#ifndef NEXUS_FILE_REPORT_SERVICE_HPP
#define NEXUS_FILE_REPORT_SERVICE_HPP
#include <filesystem>
#include <stop_token>
#include <Beam/IO/OpenState.hpp>
#include "WebPortal/ReportJobService.hpp"
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
       */
      FileReportService(std::filesystem::path definitions_directory,
        std::filesystem::path jobs_directory,
        Beam::ServiceLocatorClient client, Beam::TimeClient time_client);

      ~FileReportService();

      std::vector<ReportDefinition> load_definitions(
        const Beam::DirectoryEntry& account);
      std::string submit(const Beam::DirectoryEntry& account,
        const ReportSubmission& submission);
      void store(const ReportJob& job);
      int execute(const ReportJob& job, std::stop_token stop);
      void close();

    private:
      std::filesystem::path m_definitions_directory;
      std::filesystem::path m_jobs_directory;
      Beam::ServiceLocatorClient m_client;
      Beam::TimeClient m_time_client;
      Beam::OpenState m_open_state;
      std::unique_ptr<Details::ReportJobService<
        FileReportService, Beam::TimeClient>> m_jobs;

      FileReportService(const FileReportService&) = delete;
      FileReportService& operator =(const FileReportService&) = delete;
      std::vector<ReportDefinition> load_definitions();
      void recover();
  };
}

#endif
