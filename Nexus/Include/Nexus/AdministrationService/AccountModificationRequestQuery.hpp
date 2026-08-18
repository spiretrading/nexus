#ifndef NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_QUERY_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_QUERY_HPP
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Queries/PagedQuery.hpp>
#include <Beam/Queries/ShuttleQueryTypes.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

namespace Nexus {

  /**
   * Marks a position within an ordering of AccountModificationRequests. The
   * field used depends on the ordering being paged.
   */
  struct AccountModificationRequestAnchor {

    /** The id of the request at this position. */
    AccountModificationRequest::Id m_id;

    /** The date ordering the request. */
    boost::posix_time::ptime m_date;

    /** The account name ordering the request. */
    std::string m_name;

    /** Constructs an anchor at the first request. */
    AccountModificationRequestAnchor() noexcept;

    /**
     * Constructs an anchor.
     * @param id The id of the request at this position.
     * @param date The date ordering the request.
     * @param name The account name ordering the request.
     */
    AccountModificationRequestAnchor(AccountModificationRequest::Id id,
      boost::posix_time::ptime date, std::string name) noexcept;

    bool operator ==(const AccountModificationRequestAnchor&) const = default;
  };

  /** Queries for a page of AccountModificationRequests. */
  class AccountModificationRequestQuery : public Beam::PagedQuery<
      Beam::DirectoryEntry, AccountModificationRequestAnchor> {
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
      AccountModificationRequestQuery();

      /** Returns the field used to order the requests. */
      SortField get_sort_field() const;

      /** Sets the field used to order the requests. */
      void set_sort_field(SortField field);

      /** Returns the types to match, or empty to match every type. */
      const std::vector<AccountModificationRequest::Type>&
        get_categories() const;

      /** Sets the types to match. */
      void set_categories(
        const std::vector<AccountModificationRequest::Type>& types);

      /** Returns the statuses to match, or empty to match every status. */
      const std::vector<AccountModificationRequest::Status>&
        get_statuses() const;

      /** Sets the statuses to match. */
      void set_statuses(
        const std::vector<AccountModificationRequest::Status>& statuses);

      /** Returns the earliest update time to match. */
      const boost::optional<boost::posix_time::ptime>& get_start_date() const;

      /** Sets the earliest update time to match. */
      void set_start_date(
        const boost::optional<boost::posix_time::ptime>& date);

      /** Returns the latest update time to match. */
      const boost::optional<boost::posix_time::ptime>& get_end_date() const;

      /** Sets the latest update time to match. */
      void set_end_date(const boost::optional<boost::posix_time::ptime>& date);

      /** Returns the text matching a request id or an account name. */
      const std::string& get_search() const;

      /** Sets the text matching a request id or an account name. */
      void set_search(const std::string& search);

      /** Returns the account whose requests are excluded. */
      const boost::optional<Beam::DirectoryEntry>&
        get_excluded_account() const;

      /** Sets the account whose requests are excluded. */
      void set_excluded_account(
        const boost::optional<Beam::DirectoryEntry>& account);

    protected:
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct Beam::DataShuttle;
      SortField m_sort_field;
      std::vector<AccountModificationRequest::Type> m_categories;
      std::vector<AccountModificationRequest::Status> m_statuses;
      boost::optional<boost::posix_time::ptime> m_start_date;
      boost::optional<boost::posix_time::ptime> m_end_date;
      std::string m_search;
      boost::optional<Beam::DirectoryEntry> m_excluded_account;
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
      Beam::DirectoryEntry, AccountModificationRequestAnchor>&>(query) << ' ' <<
      query.get_sort_field() << ')';
  }

  inline AccountModificationRequestAnchor::AccountModificationRequestAnchor()
    noexcept
    : m_id(-1),
      m_date(boost::posix_time::not_a_date_time) {}

  inline AccountModificationRequestAnchor::AccountModificationRequestAnchor(
    AccountModificationRequest::Id id, boost::posix_time::ptime date,
    std::string name) noexcept
    : m_id(id),
      m_date(date),
      m_name(std::move(name)) {}

  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequestAnchor& anchor) {
    return out << '(' << anchor.m_id << ' ' << anchor.m_date << ' ' <<
      anchor.m_name << ')';
  }

  inline AccountModificationRequestQuery::AccountModificationRequestQuery()
    : m_sort_field(SortField::CREATED) {}

  inline AccountModificationRequestQuery::SortField
      AccountModificationRequestQuery::get_sort_field() const {
    return m_sort_field;
  }

  inline void AccountModificationRequestQuery::set_sort_field(SortField field) {
    m_sort_field = field;
  }

  inline const std::vector<AccountModificationRequest::Type>&
      AccountModificationRequestQuery::get_categories() const {
    return m_categories;
  }

  inline void AccountModificationRequestQuery::set_categories(
      const std::vector<AccountModificationRequest::Type>& types) {
    m_categories = types;
  }

  inline const std::vector<AccountModificationRequest::Status>&
      AccountModificationRequestQuery::get_statuses() const {
    return m_statuses;
  }

  inline void AccountModificationRequestQuery::set_statuses(
      const std::vector<AccountModificationRequest::Status>& statuses) {
    m_statuses = statuses;
  }

  inline const boost::optional<boost::posix_time::ptime>&
      AccountModificationRequestQuery::get_start_date() const {
    return m_start_date;
  }

  inline void AccountModificationRequestQuery::set_start_date(
      const boost::optional<boost::posix_time::ptime>& date) {
    m_start_date = date;
  }

  inline const boost::optional<boost::posix_time::ptime>&
      AccountModificationRequestQuery::get_end_date() const {
    return m_end_date;
  }

  inline void AccountModificationRequestQuery::set_end_date(
      const boost::optional<boost::posix_time::ptime>& date) {
    m_end_date = date;
  }

  inline const std::string&
      AccountModificationRequestQuery::get_search() const {
    return m_search;
  }

  inline void AccountModificationRequestQuery::set_search(
      const std::string& search) {
    m_search = search;
  }

  inline const boost::optional<Beam::DirectoryEntry>&
      AccountModificationRequestQuery::get_excluded_account() const {
    return m_excluded_account;
  }

  inline void AccountModificationRequestQuery::set_excluded_account(
      const boost::optional<Beam::DirectoryEntry>& account) {
    m_excluded_account = account;
  }

  template<Beam::IsShuttle S>
  void AccountModificationRequestQuery::shuttle(
      S& shuttle, unsigned int version) {
    Beam::PagedQuery<Beam::DirectoryEntry,
      AccountModificationRequestAnchor>::shuttle(shuttle, version);
    shuttle.shuttle("sort_field", m_sort_field);
    shuttle.shuttle("categories", m_categories);
    shuttle.shuttle("statuses", m_statuses);
    shuttle.shuttle("start_date", m_start_date);
    shuttle.shuttle("end_date", m_end_date);
    shuttle.shuttle("search", m_search);
    shuttle.shuttle("excluded_account", m_excluded_account);
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

namespace Beam {
  template<>
  struct Shuttle<Nexus::AccountModificationRequestAnchor> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::AccountModificationRequestAnchor& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("date", value.m_date);
      shuttle.shuttle("name", value.m_name);
    }
  };
}

#endif
