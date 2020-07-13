#ifndef NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_HPP
#define NEXUS_ADMINISTRATION_SERVICE_ACCOUNT_MODIFICATION_REQUEST_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus::AdministrationService {

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
        Beam::ServiceLocator::DirectoryEntry m_account;

        /** The update sequence number. */
        int m_sequenceNumber;

        /** The timestamp when the update occurred. */
        boost::posix_time::ptime m_timestamp;

        /** Constructs an empty Update. */
        Update() = default;

        /**
         * Constructs an Update.
         * @param status The updated status.
         * @param account The account that updated the status.
         * @param sequenceNumber The update sequence number.
         * @param timestamp The timestamp when the update occurred.
         */
        Update(Status status, Beam::ServiceLocator::DirectoryEntry account,
          int sequenceNumber, boost::posix_time::ptime timestamp);
      };

      /** Constructs an empty request. */
      AccountModificationRequest();

      /**
       * Constructs an AccountModificationRequest.
       * @param id The id that uniquely identifies this request.
       * @param type The type of modification requested.
       * @param account The account to modify.
       * @param submissionAccount The account that submitted the request.
       * @param timestamp The timestamp when the request was received.
       */
      AccountModificationRequest(Id id, Type type,
        Beam::ServiceLocator::DirectoryEntry account,
        Beam::ServiceLocator::DirectoryEntry submissionAccount,
        boost::posix_time::ptime timestamp);

      /** Returns the id that uniquely identifies this request. */
      Id GetId() const;

      /** Returns the type of modification requested. */
      Type GetType() const;

      /** Returns the account to modify. */
      const Beam::ServiceLocator::DirectoryEntry& GetAccount() const;

      /** Returns the account that submitted the request. */
      const Beam::ServiceLocator::DirectoryEntry& GetSubmissionAccount() const;

      /** Returns the timestamp when the request was received. */
      boost::posix_time::ptime GetTimestamp() const;

    private:
      friend struct Beam::Serialization::Shuttle<AccountModificationRequest>;
      Id m_id;
      Type m_type;
      Beam::ServiceLocator::DirectoryEntry m_account;
      Beam::ServiceLocator::DirectoryEntry m_submissionAccount;
      boost::posix_time::ptime m_timestamp;
  };

  /** Returns <code>true</code> iff a status represents a terminal state. */
  inline bool IsTerminal(AccountModificationRequest::Status status) {
    return status == AccountModificationRequest::Status::GRANTED ||
      status == AccountModificationRequest::Status::REJECTED;
  }

  inline AccountModificationRequest::Update::Update(Status status,
    Beam::ServiceLocator::DirectoryEntry account, int sequenceNumber,
    boost::posix_time::ptime timestamp)
    : m_status(status),
      m_account(std::move(account)),
      m_sequenceNumber(sequenceNumber),
      m_timestamp(timestamp) {}

  inline AccountModificationRequest::AccountModificationRequest()
    : m_id(-1),
      m_type(Type::ENTITLEMENTS) {}

  inline AccountModificationRequest::AccountModificationRequest(Id id,
    Type type, Beam::ServiceLocator::DirectoryEntry account,
    Beam::ServiceLocator::DirectoryEntry submissionAccount,
    boost::posix_time::ptime timestamp)
    : m_id(id),
      m_type(type),
      m_account(std::move(account)),
      m_submissionAccount(std::move(submissionAccount)),
      m_timestamp(timestamp) {}

  inline AccountModificationRequest::Id
      AccountModificationRequest::GetId() const {
    return m_id;
  }

  inline AccountModificationRequest::Type
      AccountModificationRequest::GetType() const {
    return m_type;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      AccountModificationRequest::GetAccount() const {
    return m_account;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      AccountModificationRequest::GetSubmissionAccount() const {
    return m_submissionAccount;
  }

  inline boost::posix_time::ptime
      AccountModificationRequest::GetTimestamp() const {
    return m_timestamp;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<
      Nexus::AdministrationService::AccountModificationRequest::Update> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::AccountModificationRequest::Update& value,
        unsigned int version) {
      shuttle.Shuttle("status", value.m_status);
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("sequence_number", value.m_sequenceNumber);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };

  template<>
  struct Shuttle<Nexus::AdministrationService::AccountModificationRequest> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::AccountModificationRequest& value,
        unsigned int version) {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("submission_account", value.m_submissionAccount);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
