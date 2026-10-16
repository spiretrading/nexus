#ifndef NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/ToString.hpp>
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
        RISK,

        /** Modify an account's compliance parameters. */
        COMPLIANCE
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
       * @param effective_date The date when the modification should take effect.
       */
      AccountModificationRequest(Id id, Type type, Beam::DirectoryEntry account,
        Beam::DirectoryEntry submission_account,
        boost::posix_time::ptime timestamp,
        boost::posix_time::ptime effective_date) noexcept;

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

      /** Returns the date when the modification should take effect. */
      boost::posix_time::ptime get_effective_date() const;

      bool operator ==(const AccountModificationRequest&) const = default;

    private:
      friend struct Beam::Shuttle<AccountModificationRequest>;
      Id m_id;
      Type m_type;
      Beam::DirectoryEntry m_account;
      Beam::DirectoryEntry m_submission_account;
      boost::posix_time::ptime m_timestamp;
      boost::posix_time::ptime m_effective_date;
  };

  /** Returns <code>true</code> iff a status represents a terminal state. */
  inline bool is_terminal(AccountModificationRequest::Status status) {
    return status == AccountModificationRequest::Status::GRANTED ||
      status == AccountModificationRequest::Status::REJECTED;
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
    boost::posix_time::ptime timestamp,
    boost::posix_time::ptime effective_date) noexcept
    : m_id(id),
      m_type(type),
      m_account(std::move(account)),
      m_submission_account(std::move(submission_account)),
      m_timestamp(timestamp),
      m_effective_date(effective_date) {}

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

  inline boost::posix_time::ptime
      AccountModificationRequest::get_effective_date() const {
    return m_effective_date;
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
      shuttle.shuttle("effective_date", value.m_effective_date);
    }
  };
}

template<>
class std::formatter<Nexus::AccountModificationRequest::Type> :
    public std::formatter<std::string_view> {
  public:
    auto format(
        Nexus::AccountModificationRequest::Type value, auto& context) const {
      auto text = [&] {
        if(value == Nexus::AccountModificationRequest::Type::ENTITLEMENTS) {
          return "ENTITLEMENTS";
        } else if(value == Nexus::AccountModificationRequest::Type::RISK) {
          return "RISK";
        } else if(value == Nexus::AccountModificationRequest::Type::COMPLIANCE) {
          return "COMPLIANCE";
        }
        return "UNKNOWN";
      }();
      return std::formatter<std::string_view>::format(text, context);
    }
};

template<>
class std::formatter<Nexus::AccountModificationRequest::Status> :
    public std::formatter<std::string_view> {
  public:
    auto format(
        Nexus::AccountModificationRequest::Status value, auto& context) const {
      auto text = [&] {
        if(value == Nexus::AccountModificationRequest::Status::NONE) {
          return "NONE";
        } else if(value == Nexus::AccountModificationRequest::Status::PENDING) {
          return "PENDING";
        } else if(value == Nexus::AccountModificationRequest::Status::REVIEWED) {
          return "REVIEWED";
        } else if(
            value == Nexus::AccountModificationRequest::Status::SCHEDULED) {
          return "SCHEDULED";
        } else if(value == Nexus::AccountModificationRequest::Status::GRANTED) {
          return "GRANTED";
        } else if(value == Nexus::AccountModificationRequest::Status::REJECTED) {
          return "REJECTED";
        }
        return "UNKNOWN";
      }();
      return std::formatter<std::string_view>::format(text, context);
    }
};

template<>
class std::formatter<Nexus::AccountModificationRequest::Update> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::AccountModificationRequest::Update& value,
        auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {})", value.m_status,
          value.m_account, value.m_sequence_number,
          Beam::to_string(value.m_timestamp)), context);
    }
};

template<>
class std::formatter<Nexus::AccountModificationRequest> :
    public std::formatter<std::string_view> {
  public:
    auto format(
        const Nexus::AccountModificationRequest& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} {} {} {} {})", value.get_id(),
          value.get_type(), value.get_account(), value.get_submission_account(),
          Beam::to_string(value.get_timestamp()),
          Beam::to_string(value.get_effective_date())), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, AccountModificationRequest::Type value) {
    return out << std::format(out.getloc(), "{}", value);
  }

  inline std::ostream& operator <<(
      std::ostream& out, AccountModificationRequest::Status value) {
    return out << std::format(out.getloc(), "{}", value);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequest::Update& value) {
    return out << std::format(out.getloc(), "{}", value);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const AccountModificationRequest& value) {
    return out << std::format(out.getloc(), "{}", value);
  }
}

#endif
