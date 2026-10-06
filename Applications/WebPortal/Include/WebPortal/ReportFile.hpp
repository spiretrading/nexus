#ifndef NEXUS_REPORT_FILE_HPP
#define NEXUS_REPORT_FILE_HPP
#include <filesystem>
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** A generated report's downloadable file. */
  struct ReportFile {

    /** The filename presented to the user. */
    std::filesystem::path m_name;

    /** The configured MIME type. */
    std::string m_media_type;

    /** The generated file's bytes. */
    Beam::SharedBuffer m_content;
  };

  /** Formats a filename from the report type, job id, and extension. */
  std::filesystem::path make_report_filename(const ReportJob& job);
}

#endif
