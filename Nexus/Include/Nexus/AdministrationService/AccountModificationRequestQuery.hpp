#ifndef NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_QUERY_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_QUERY_HPP
#include <Beam/Queries/PagedQuery.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

namespace Nexus {

  /** Defines the type of query used to load AccountModificationRequests. */
  using AccountModificationRequestQuery =
    Beam::PagedQuery<Beam::DirectoryEntry, AccountModificationRequest::Id>;

  /**
   * Returns a query for an index's most recent AccountModificationRequests.
   * @param index The account or directory whose requests are to be queried.
   * @param count The maximum number of requests to query.
   * @return A query for the <i>index</i>'s most recent requests.
   */
  inline AccountModificationRequestQuery
      make_account_modification_request_query(
        const Beam::DirectoryEntry& index, int count) {
    auto query = AccountModificationRequestQuery();
    query.set_index(index);
    query.set_snapshot_limit(Beam::SnapshotLimit::from_tail(count));
    return query;
  }
}

#endif
