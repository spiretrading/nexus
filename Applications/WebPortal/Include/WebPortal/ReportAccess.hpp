#ifndef NEXUS_REPORT_ACCESS_HPP
#define NEXUS_REPORT_ACCESS_HPP
#include <unordered_set>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** Checks report visibility and access using current group memberships. */
  class ReportAccess {
    public:

      /**
       * Constructs an access checker for one request.
       * @param account The account requesting reports.
       * @param client The client for group membership lookups.
       */
      ReportAccess(const Beam::DirectoryEntry& account,
        Beam::ServiceLocatorClient client);

      /** Tests whether the account can view the job's metadata and status. */
      bool is_visible(const ReportJob& job);

      /** Tests whether the account can access a completed report's file. */
      bool is_accessible(const ReportJob& job);

    private:
      Beam::DirectoryEntry m_account;
      Beam::ServiceLocatorClient m_client;
      std::optional<std::unordered_set<Beam::DirectoryEntry>> m_groups;
  };
}

#endif
