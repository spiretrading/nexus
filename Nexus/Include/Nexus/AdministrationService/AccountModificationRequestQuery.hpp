#ifndef NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_QUERY_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_QUERY_HPP
#include <ostream>
#include <Beam/Queries/PagedQuery.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

namespace Nexus {

  /** Queries for a page of AccountModificationRequests. */
  class AccountModificationRequestQuery : public Beam::PagedQuery<
      Beam::DirectoryEntry, AccountModificationRequest::Id> {
    public:

      /** Lists the fields that requests can be ordered by. */
      enum class SortField {

        /** Order by the time a request was submitted. */
        CREATED,

        /** Order by the time a request was last updated. */
        LAST_UPDATED,

        /** Order by the date a request takes effect. */
        EFFECTIVE_DATE,

        /** Order by the name of the account being modified. */
        ACCOUNT,

        /** Order by the name of the account that submitted the request. */
        REQUESTER
      };

      /** Constructs a query ordered by the time a request was submitted. */
      AccountModificationRequestQuery() noexcept;

      /** Returns the field used to order the requests. */
      SortField get_sort_field() const;

      /** Sets the field used to order the requests. */
      void set_sort_field(SortField field);

    protected:
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct Beam::DataShuttle;
      SortField m_sort_field;
  };

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

  inline std::ostream& operator <<(
      std::ostream& out, AccountModificationRequestQuery::SortField field) {
    if(field == AccountModificationRequestQuery::SortField::LAST_UPDATED) {
      return out << "LAST_UPDATED";
    } else if(
        field == AccountModificationRequestQuery::SortField::EFFECTIVE_DATE) {
      return out << "EFFECTIVE_DATE";
    } else if(field == AccountModificationRequestQuery::SortField::ACCOUNT) {
      return out << "ACCOUNT";
    } else if(field == AccountModificationRequestQuery::SortField::REQUESTER) {
      return out << "REQUESTER";
    }
    return out << "CREATED";
  }

  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequestQuery& query) {
    return out << '(' << static_cast<const Beam::PagedQuery<
      Beam::DirectoryEntry, AccountModificationRequest::Id>&>(query) << ' ' <<
      query.get_sort_field() << ')';
  }

  inline AccountModificationRequestQuery::
    AccountModificationRequestQuery() noexcept
    : m_sort_field(SortField::CREATED) {}

  inline AccountModificationRequestQuery::SortField
      AccountModificationRequestQuery::get_sort_field() const {
    return m_sort_field;
  }

  inline void AccountModificationRequestQuery::set_sort_field(
      SortField field) {
    m_sort_field = field;
  }

  template<Beam::IsShuttle S>
  void AccountModificationRequestQuery::shuttle(
      S& shuttle, unsigned int version) {
    Beam::PagedQuery<Beam::DirectoryEntry,
      AccountModificationRequest::Id>::shuttle(shuttle, version);
    shuttle.shuttle("sort_field", m_sort_field);
    if constexpr(Beam::IsReceiver<S>) {
      if(m_sort_field != SortField::CREATED &&
          m_sort_field != SortField::LAST_UPDATED &&
          m_sort_field != SortField::EFFECTIVE_DATE &&
          m_sort_field != SortField::ACCOUNT &&
          m_sort_field != SortField::REQUESTER) {
        m_sort_field = SortField::CREATED;
      }
    }
  }
}

#endif
