#ifndef NEXUS_REPORT_ACCESS_HPP
#define NEXUS_REPORT_ACCESS_HPP
#include <unordered_set>
#include "WebPortal/ReportJob.hpp"

namespace Nexus {

  /** Checks access to completed reports using current group memberships. */
  class ReportAccess {
    public:

      /**
       * Constructs an access checker for one request.
       * @param account The account requesting reports.
       * @param client The client for group membership lookups.
       */
      ReportAccess(const Beam::DirectoryEntry& account,
        Beam::ServiceLocatorClient client);

      /** Tests whether a report is owned by or shared with the account. */
      bool is_accessible(const ReportJob& job);

    private:
      Beam::DirectoryEntry m_account;
      Beam::ServiceLocatorClient m_client;
      std::optional<std::unordered_set<Beam::DirectoryEntry>> m_groups;
  };
}

#endif
