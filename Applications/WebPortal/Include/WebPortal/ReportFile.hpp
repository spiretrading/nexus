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

  /** Returns the saved download filename or formats its initial name. */
  std::filesystem::path make_report_filename(const ReportJob& job);

  /** Formats a download filename with a suffix to avoid existing names. */
  std::filesystem::path make_report_filename(
    const ReportJob& job, std::span<const std::string> filenames);
}

#endif
