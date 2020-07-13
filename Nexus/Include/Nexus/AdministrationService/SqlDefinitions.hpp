#ifndef NEXUS_ADMINISTRATION_SERVICE_SQL_DEFINITIONS_HPP
#define NEXUS_ADMINISTRATION_SERVICE_SQL_DEFINITIONS_HPP
#include <Beam/Sql/Conversions.hpp>
#include <Beam/Sql/PosixTimeToSqlDateTime.hpp>
#include <Viper/Row.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/Definitions/SqlDefinitions.hpp"

namespace Nexus::AdministrationService {

  /** Returns a row representing an AccountIdentity. */
  inline const auto& GetAccountIdentityRow() {
    static auto ROW = Viper::Row<AccountIdentity>().
      add_column("first_name", Viper::varchar(64),
        &AccountIdentity::m_firstName).
      add_column("last_name", Viper::varchar(64), &AccountIdentity::m_lastName).
      add_column("e_mail", Viper::varchar(64),
        &AccountIdentity::m_emailAddress).
      add_column("address_line_one", Viper::varchar(256),
         &AccountIdentity::m_addressLineOne).
      add_column("address_line_two", Viper::varchar(256),
        &AccountIdentity::m_addressLineTwo).
      add_column("address_line_three", Viper::varchar(256),
        &AccountIdentity::m_addressLineThree).
      add_column("city", Viper::varchar(64), &AccountIdentity::m_city).
      add_column("province", Viper::varchar(64), &AccountIdentity::m_province).
      add_column("country", &AccountIdentity::m_country).
      add_column("user_notes", Viper::text, &AccountIdentity::m_userNotes);
    return ROW;
  }

  /** Returns a row representing an IndexedAccountIdentity. */
  inline const auto& GetIndexedAccountIdentityRow() {
    static auto ROW = Viper::Row<
      AdministrationDataStore::IndexedAccountIdentity>().
      add_column("account",
        [] (const auto& row) {
          return std::get<0>(row).m_id;
        },
        [] (auto& row, auto column) {
          std::get<0>(row) = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
            column);
        }).
      extend(GetAccountIdentityRow(),
        [] (auto& row) -> auto& {
          return std::get<1>(row);
        }).
      add_column("photo_id", Viper::varchar(256),
        [] (const auto& row) {
          return std::string();
        },
        [] (auto& row, auto column) {}).
      set_primary_key("account");
    return ROW;
  }

  /** Returns a row representing RiskParameters. */
  inline const auto& GetRiskParametersRow() {
    static auto ROW = Viper::Row<RiskService::RiskParameters>().
      add_column("currency", &RiskService::RiskParameters::m_currency).
      add_column("buying_power", &RiskService::RiskParameters::m_buyingPower).
      add_column("net_loss", &RiskService::RiskParameters::m_netLoss).
      add_column("allowed_state",
        [] (auto& row) -> auto& {
          return row.m_allowedState.m_type;
        }).
      add_column("loss_from_top", &RiskService::RiskParameters::m_lossFromTop).
      add_column("transition_time",
        &RiskService::RiskParameters::m_transitionTime);
    return ROW;
  }

  /** Returns a row representing an account's RiskParameters. */
  inline const auto& GetIndexedRiskParametersRow() {
    static auto ROW = Viper::Row<
      AdministrationDataStore::IndexedRiskParameters>().
      add_column("account",
        [] (const auto& row) {
          return std::get<0>(row).m_id;
        },
        [] (auto& row, auto column) {
          std::get<0>(row) = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
            column);
        }).
      extend(GetRiskParametersRow(),
        [] (auto& row) -> auto& {
          return std::get<1>(row);
        }).
      set_primary_key("account");
    return ROW;
  }

  /** Returns a row representing a RiskState. */
  inline const auto& GetRiskStateRow() {
    static auto ROW = Viper::Row<RiskService::RiskState>().
      add_column("state", &RiskService::RiskState::m_type).
      add_column("expiry", &RiskService::RiskState::m_expiry);
    return ROW;
  }

  /** Returns a row representing an IndexedRiskState. */
  inline const auto& GetIndexedRiskStateRow() {
    static auto ROW = Viper::Row<AdministrationDataStore::IndexedRiskState>().
      add_column("account",
        [] (const auto& row) {
          return std::get<0>(row).m_id;
        },
        [] (auto& row, auto column) {
          std::get<0>(row) = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
            column);
        }).
      extend(GetRiskStateRow(),
        [] (auto& row) -> auto& {
          return std::get<1>(row);
        }).
      set_primary_key("account");
    return ROW;
  }

  /** Returns a row representing an AccountModificationRequest. */
  inline const auto& GetAccountModificationRequestRow() {
    static auto ROW = Viper::Row<AccountModificationRequest>().
      add_column("id",
        [] (const auto& row) {
          return row.GetId();
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(column, row.GetType(),
            row.GetAccount(), row.GetSubmissionAccount(), row.GetTimestamp());
        }).
      add_column("type",
        [] (const auto& row) {
          return row.GetType();
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.GetId(), column,
            row.GetAccount(), row.GetSubmissionAccount(), row.GetTimestamp());
        }).
      add_column("account",
        [] (const auto& row) {
          return row.GetAccount().m_id;
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.GetId(), row.GetType(),
            Beam::ServiceLocator::DirectoryEntry::MakeAccount(column),
            row.GetSubmissionAccount(), row.GetTimestamp());
        }).
      add_column("submission_account",
        [] (const auto& row) {
          return row.GetSubmissionAccount().m_id;
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.GetId(), row.GetType(),
            row.GetAccount(), Beam::ServiceLocator::DirectoryEntry::MakeAccount(
            column), row.GetTimestamp());
        }).
      add_column("timestamp",
        [] (const auto& row) {
          return row.GetTimestamp();
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.GetId(), row.GetType(),
            row.GetAccount(), row.GetSubmissionAccount(), column);
        }).
      set_primary_key("id").
      add_index("account_index", {"id", "account"});
    return ROW;
  }

  /** Represents a row representing a single entitlement modification. */
  struct EntitlementModificationRow {

    /** The modification's id. */
    int m_id;

    /** The entitlement to grant. */
    Beam::ServiceLocator::DirectoryEntry m_entitlement;
  };

  /** Returns a row representing a single entitlement modification. */
  inline const auto& GetEntitlementModificationRow() {
    static const auto ROW = Viper::Row<EntitlementModificationRow>().
      add_column("id", &EntitlementModificationRow::m_id).
      add_column("entitlement",
        [] (const auto& row) {
          return row.m_entitlement.m_id;
        },
        [] (auto& row, auto column) {
          row.m_entitlement =
            Beam::ServiceLocator::DirectoryEntry::MakeDirectory(column);
        }).
      add_index("id_index", "id");
    return ROW;
  }

  /** Represents an indexed RiskModification. */
  struct IndexedRiskModification {

    /** The request's id. */
    int m_id;

    /** The account being modified. */
    Beam::ServiceLocator::DirectoryEntry m_account;

    /** The requested parameters. */
    RiskService::RiskParameters m_parameters;
  };

  /** Returns a row representing a RiskModification. */
  inline const auto& GetRiskModificationRow() {
    static const auto ROW = Viper::Row<IndexedRiskModification>().
      add_column("id", &IndexedRiskModification::m_id).
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
            column);
        }).
      extend(GetRiskParametersRow(), &IndexedRiskModification::m_parameters).
      set_primary_key("id");
    return ROW;
  }

  /** Returns a row representing an AccountModificationRequest's status. */
  inline const auto& GetAccountModificationRequestStatusRow() {
    static const auto ROW = Viper::Row<AccountModificationRequest::Update>().
      add_column("status", &AccountModificationRequest::Update::m_status).
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
            column);
        }).
      add_column("sequence_number",
        &AccountModificationRequest::Update::m_sequenceNumber).
      add_column("timestamp", &AccountModificationRequest::Update::m_timestamp);
    return ROW;
  }

  /** Represents an indexed status of an account modification request. */
  struct IndexedAccountModificationRequestStatus {

    /** The request's id. */
    int m_id;

    /** The update representing the status. */
    AccountModificationRequest::Update m_update;
  };

  /** Returns a row representring an IndexedAccountModificationRequestStatus. */
  inline const auto& GetIndexedAccountModificationRequestStatus() {
    static const auto ROW = Viper::Row<
      IndexedAccountModificationRequestStatus>().
      add_column("id", &IndexedAccountModificationRequestStatus::m_id).
      extend(GetAccountModificationRequestStatusRow(),
        &IndexedAccountModificationRequestStatus::m_update).
      add_index("id_index", "id");
    return ROW;
  }

  /** Represents the indicies used to access messages. */
  struct AdministrationMessageIndex {

    /** The message id. */
    int m_id;

    /** The account that sent the message. */
    Beam::ServiceLocator::DirectoryEntry m_account;

    /** The message's timestamp. */
    boost::posix_time::ptime m_timestamp;
  };

  /** Returns a row representing an AdministrationMessageIndex. */
  inline const auto& GetAdministrationMessageIndexRow() {
    static const auto ROW = Viper::Row<AdministrationMessageIndex>().
      add_column("id", &AdministrationMessageIndex::m_id).
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
            column);
        }).
      add_column("timestamp", &AdministrationMessageIndex::m_timestamp).
      set_primary_key("id");
    return ROW;
  }

  /** Returns a row representing a Message::Body. */
  inline const auto& GetMessageBodyRow() {
    static const auto ROW = Viper::Row<Message::Body>().
      add_column("content_type", Viper::varchar(100),
        &Message::Body::m_contentType).
      add_column("message", Viper::text, &Message::Body::m_message);
    return ROW;
  }

  /** Represents an indexed message body. */
  struct IndexedMessageBody {

    /** The Message id. */
    int m_id;

    /** The Message Body. */
    Message::Body m_body;
  };

  /** Returns a row representing a message body. */
  inline const auto& GetIndexedMessageBodyRow() {
    static const auto ROW = Viper::Row<IndexedMessageBody>().
      add_column("id", &IndexedMessageBody::m_id).
      extend(GetMessageBodyRow(), &IndexedMessageBody::m_body).
      add_index("id_index", "id");
    return ROW;
  }

  /** Represents an index to an administration message. */
  struct AccountModificationRequestMessageIndex {

    /** The id of the request the message belongs to. */
    int m_requestId;

    /** The id of the message. */
    int m_messageId;
  };

  /** Returns a row representing a message body. */
  inline const auto& GetAccountModificationRequestMessageIndexRow() {
    static const auto ROW = Viper::Row<
      AccountModificationRequestMessageIndex>().
      add_column("request_id",
        &AccountModificationRequestMessageIndex::m_requestId).
      add_column("message_id",
        &AccountModificationRequestMessageIndex::m_messageId).
      add_index("id_index", "request_id");
    return ROW;
  }
}

#endif
