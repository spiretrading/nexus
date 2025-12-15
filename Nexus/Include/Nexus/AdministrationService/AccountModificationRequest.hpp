#ifndef NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Nexus {

  /** Stores a request to modify an account. */
  class AccountModificationRequest {
    public:

      /** The type used to uniquely identify a request. */
      using Id = int;

      /** Lists the modifications that can be made to an account. */
      enum class Type {

        /** Modify an account's market data entitlements. */
        ENTITLEMENTS,

        /** Modify an account's risk parameters. */
        RISK
      };

      /** Lists the status of a request. */
      enum class Status {

        /** No or invalid status. */
        NONE,

        /** The request is pending. */
        PENDING,

        /** The request has been reviewed by a manager. */
        REVIEWED,

        /** The request has been scheduled. */
        SCHEDULED,

        /** The request has been granted. */
        GRANTED,

        /** The request has been rejected. */
        REJECTED
      };

      /** Stores a request status update. */
      struct Update {

        /** The updated status. */
        Status m_status;

        /** The account that updated the status. */
        Beam::DirectoryEntry m_account;

        /** The update sequence number. */
        int m_sequence_number;

        /** The timestamp when the update occurred. */
        boost::posix_time::ptime m_timestamp;

        /** Constructs an empty Update. */
        Update() noexcept;

        /**
         * Constructs an Update.
         * @param status The updated status.
         * @param account The account that updated the status.
         * @param sequence_number The update sequence number.
         * @param timestamp The timestamp when the update occurred.
         */
        Update(Status status, Beam::DirectoryEntry account, int sequence_number,
          boost::posix_time::ptime timestamp) noexcept;

        bool operator ==(const Update&) const = default;
      };

      /** Constructs an empty request. */
      AccountModificationRequest() noexcept;

      /**
       * Constructs an AccountModificationRequest.
       * @param id The id that uniquely identifies this request.
       * @param type The type of modification requested.
       * @param account The account to modify.
       * @param submission_account The account that submitted the request.
       * @param timestamp The timestamp when the request was received.
       */
      AccountModificationRequest(Id id, Type type, Beam::DirectoryEntry account,
        Beam::DirectoryEntry submission_account,
        boost::posix_time::ptime timestamp) noexcept;

      /** Returns the id that uniquely identifies this request. */
      Id get_id() const;

      /** Returns the type of modification requested. */
      Type get_type() const;

      /** Returns the account to modify. */
      const Beam::DirectoryEntry& get_account() const;

      /** Returns the account that submitted the request. */
      const Beam::DirectoryEntry& get_submission_account() const;

      /** Returns the timestamp when the request was received. */
      boost::posix_time::ptime get_timestamp() const;

    private:
      friend struct Beam::Shuttle<AccountModificationRequest>;
      Id m_id;
      Type m_type;
      Beam::DirectoryEntry m_account;
      Beam::DirectoryEntry m_submission_account;
      boost::posix_time::ptime m_timestamp;
  };

  /** Returns <code>true</code> iff a status represents a terminal state. */
  inline bool is_terminal(AccountModificationRequest::Status status) {
    return status == AccountModificationRequest::Status::GRANTED ||
      status == AccountModificationRequest::Status::REJECTED;
  }

  inline std::ostream& operator <<(
      std::ostream& out, AccountModificationRequest::Type type) {
    if(type == AccountModificationRequest::Type::ENTITLEMENTS) {
      return out << "ENTITLEMENTS";
    } else if(type == AccountModificationRequest::Type::RISK) {
      return out << "RISK";
    } else {
      return out << "UNKNOWN";
    }
  }

  inline std::ostream& operator <<(
      std::ostream& out, AccountModificationRequest::Status status) {
    if(status == AccountModificationRequest::Status::NONE) {
      return out << "NONE";
    } else if(status == AccountModificationRequest::Status::PENDING) {
      return out << "PENDING";
    } else if(status == AccountModificationRequest::Status::REVIEWED) {
      return out << "REVIEWED";
    } else if(status == AccountModificationRequest::Status::SCHEDULED) {
      return out << "SCHEDULED";
    } else if(status == AccountModificationRequest::Status::GRANTED) {
      return out << "GRANTED";
    } else if(status == AccountModificationRequest::Status::REJECTED) {
      return out << "REJECTED";
    } else {
      return out << "UNKNOWN";
    }
  }

  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequest::Update& update) {
    return out << '(' << update.m_status << ' ' << update.m_account << ' '
      << update.m_sequence_number << ' ' << update.m_timestamp << ')';
  }

  inline AccountModificationRequest::Update::Update() noexcept
    : Update(AccountModificationRequest::Status::NONE, Beam::DirectoryEntry(),
        -1, boost::posix_time::not_a_date_time) {}

  inline AccountModificationRequest::Update::Update(
    Status status, Beam::DirectoryEntry account, int sequence_number,
    boost::posix_time::ptime timestamp) noexcept
    : m_status(status),
      m_account(std::move(account)),
      m_sequence_number(sequence_number),
      m_timestamp(timestamp) {}

  inline AccountModificationRequest::AccountModificationRequest() noexcept
    : m_id(-1),
      m_type(Type::ENTITLEMENTS) {}

  inline AccountModificationRequest::AccountModificationRequest(
    Id id, Type type, Beam::DirectoryEntry account,
    Beam::DirectoryEntry submission_account,
    boost::posix_time::ptime timestamp) noexcept
    : m_id(id),
      m_type(type),
      m_account(std::move(account)),
      m_submission_account(std::move(submission_account)),
      m_timestamp(timestamp) {}

  inline AccountModificationRequest::Id
      AccountModificationRequest::get_id() const {
    return m_id;
  }

  inline AccountModificationRequest::Type
      AccountModificationRequest::get_type() const {
    return m_type;
  }

  inline const Beam::DirectoryEntry&
      AccountModificationRequest::get_account() const {
    return m_account;
  }

  inline const Beam::DirectoryEntry&
      AccountModificationRequest::get_submission_account() const {
    return m_submission_account;
  }

  inline boost::posix_time::ptime
      AccountModificationRequest::get_timestamp() const {
    return m_timestamp;
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::AccountModificationRequest::Update> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::AccountModificationRequest::Update& value,
        unsigned int version) const {
      shuttle.shuttle("status", value.m_status);
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("sequence_number", value.m_sequence_number);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };

  template<>
  struct Shuttle<Nexus::AccountModificationRequest> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::AccountModificationRequest& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("submission_account", value.m_submission_account);
      shuttle.shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
