#ifndef NEXUS_ADMINISTRATION_SERVICE_SQL_DEFINITIONS_HPP
#define NEXUS_ADMINISTRATION_SERVICE_SQL_DEFINITIONS_HPP
#include <Beam/Sql/Conversions.hpp>
#include <Beam/Sql/PosixTimeToSqlDateTime.hpp>
#include <Viper/Row.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/Definitions/SqlDefinitions.hpp"

namespace Nexus {

  /** Returns a row representing an AccountIdentity. */
  inline const auto& get_account_identity_row() {
    static auto ROW = Viper::Row<AccountIdentity>().
      add_column(
        "first_name", Viper::varchar(64), &AccountIdentity::m_first_name).
      add_column(
        "last_name", Viper::varchar(64), &AccountIdentity::m_last_name).
      add_column(
        "e_mail", Viper::varchar(64), &AccountIdentity::m_email_address).
      add_column("address_line_one", Viper::varchar(256),
        &AccountIdentity::m_address_line_one).
      add_column("address_line_two", Viper::varchar(256),
        &AccountIdentity::m_address_line_two).
      add_column("address_line_three", Viper::varchar(256),
        &AccountIdentity::m_address_line_three).
      add_column("city", Viper::varchar(64), &AccountIdentity::m_city).
      add_column("province", Viper::varchar(64), &AccountIdentity::m_province).
      add_column("country", &AccountIdentity::m_country).
      add_column("user_notes", Viper::text, &AccountIdentity::m_user_notes);
    return ROW;
  }

  /** Returns a row representing an IndexedAccountIdentity. */
  inline const auto& get_indexed_account_identity_row() {
    static auto ROW =
        Viper::Row<AdministrationDataStore::IndexedAccountIdentity>().
      add_column("account",
        [] (const auto& row) {
          return row.m_index.m_id;
        },
        [] (auto& row, auto column) {
          row.m_index = Beam::DirectoryEntry::make_account(column);
        }).
      extend(get_account_identity_row(),
        [] (auto& row) -> auto& {
          return row.m_identity;
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
  inline const auto& get_risk_parameters_row() {
    static auto ROW = Viper::Row<RiskParameters>().
      add_column("currency", &RiskParameters::m_currency).
      add_column("buying_power", &RiskParameters::m_buying_power).
      add_column("net_loss", &RiskParameters::m_net_loss).
      add_column("allowed_state",
        [] (auto& row) -> auto& {
          return row.m_allowed_state.m_type;
        }).
      add_column("transition_time", &RiskParameters::m_transition_time);
    return ROW;
  }

  /** Returns a row representing an account's RiskParameters. */
  inline const auto& get_indexed_risk_parameters_row() {
    static auto ROW =
        Viper::Row<AdministrationDataStore::IndexedRiskParameters>().
      add_column("account",
        [] (const auto& row) {
          return row.m_index.m_id;
        },
        [] (auto& row, auto column) {
          row.m_index = Beam::DirectoryEntry::make_account(column);
        }).
      extend(get_risk_parameters_row(),
        [] (auto& row) -> auto& {
          return row.m_parameters;
        }).
      set_primary_key("account");
    return ROW;
  }

  /** Returns a row representing a RiskState. */
  inline const auto& get_risk_state_row() {
    static auto ROW = Viper::Row<RiskState>().
      add_column("state", &RiskState::m_type).
      add_column("expiry", &RiskState::m_expiry);
    return ROW;
  }

  /** Returns a row representing an IndexedRiskState. */
  inline const auto& get_indexed_risk_state_row() {
    static auto ROW = Viper::Row<AdministrationDataStore::IndexedRiskState>().
      add_column("account",
        [] (const auto& row) {
          return row.m_index.m_id;
        },
        [] (auto& row, auto column) {
          row.m_index = Beam::DirectoryEntry::make_account(column);
        }).
      extend(get_risk_state_row(),
        [] (auto& row) -> auto& {
          return row.m_state;
        }).
      set_primary_key("account");
    return ROW;
  }

  /** Returns a row representing an AccountModificationRequest. */
  inline const auto& get_account_modification_request_row() {
    static auto ROW = Viper::Row<AccountModificationRequest>().
      add_column("id",
        [] (const auto& row) {
          return row.get_id();
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(column, row.get_type(),
            row.get_account(), row.get_submission_account(),
            row.get_timestamp());
        }).
      add_column("type",
        [] (const auto& row) {
          return row.get_type();
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.get_id(), column,
            row.get_account(), row.get_submission_account(),
            row.get_timestamp());
        }).
      add_column("account",
        [] (const auto& row) {
          return row.get_account().m_id;
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.get_id(), row.get_type(),
            Beam::DirectoryEntry::make_account(column),
            row.get_submission_account(), row.get_timestamp());
        }).
      add_column("submission_account",
        [] (const auto& row) {
          return row.get_submission_account().m_id;
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.get_id(), row.get_type(),
            row.get_account(), Beam::DirectoryEntry::make_account(column),
            row.get_timestamp());
        }).
      add_column("timestamp",
        [] (const auto& row) {
          return row.get_timestamp();
        },
        [] (auto& row, auto column) {
          row = AccountModificationRequest(row.get_id(), row.get_type(),
            row.get_account(), row.get_submission_account(), column);
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
    Beam::DirectoryEntry m_entitlement;
  };

  /** Returns a row representing a single entitlement modification. */
  inline const auto& get_entitlement_modification_row() {
    static const auto ROW = Viper::Row<EntitlementModificationRow>().
      add_column("id", &EntitlementModificationRow::m_id).
      add_column("entitlement",
        [] (const auto& row) {
          return row.m_entitlement.m_id;
        },
        [] (auto& row, auto column) {
          row.m_entitlement = Beam::DirectoryEntry::make_directory(column);
        }).
      add_index("id_index", "id");
    return ROW;
  }

  /** Represents an indexed RiskModification. */
  struct IndexedRiskModification {

    /** The request's id. */
    int m_id;

    /** The account being modified. */
    Beam::DirectoryEntry m_account;

    /** The requested parameters. */
    RiskParameters m_parameters;
  };

  /** Returns a row representing a RiskModification. */
  inline const auto& get_risk_modification_row() {
    static const auto ROW = Viper::Row<IndexedRiskModification>().
      add_column("id", &IndexedRiskModification::m_id).
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_account = Beam::DirectoryEntry::make_account(column);
        }).
      extend(get_risk_parameters_row(), &IndexedRiskModification::m_parameters).
      set_primary_key("id");
    return ROW;
  }

  /** Returns a row representing an AccountModificationRequest's status. */
  inline const auto& get_account_modification_request_status_row() {
    static const auto ROW = Viper::Row<AccountModificationRequest::Update>().
      add_column("status", &AccountModificationRequest::Update::m_status).
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_account = Beam::DirectoryEntry::make_account(column);
        }).
      add_column("sequence_number",
        &AccountModificationRequest::Update::m_sequence_number).
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

  /** Returns a row representing an IndexedAccountModificationRequestStatus. */
  inline const auto& get_indexed_account_modification_request_status() {
    static const auto ROW =
      Viper::Row<IndexedAccountModificationRequestStatus>().
        add_column("id", &IndexedAccountModificationRequestStatus::m_id).
        extend(get_account_modification_request_status_row(),
          &IndexedAccountModificationRequestStatus::m_update).
        add_index("id_index", "id");
    return ROW;
  }

  /** Represents the indices used to access messages. */
  struct AdministrationMessageIndex {

    /** The message id. */
    int m_id;

    /** The account that sent the message. */
    Beam::DirectoryEntry m_account;

    /** The message's timestamp. */
    boost::posix_time::ptime m_timestamp;
  };

  /** Returns a row representing an AdministrationMessageIndex. */
  inline const auto& get_administration_message_index_row() {
    static const auto ROW = Viper::Row<AdministrationMessageIndex>().
      add_column("id", &AdministrationMessageIndex::m_id).
      add_column("account",
        [] (const auto& row) {
          return row.m_account.m_id;
        },
        [] (auto& row, auto column) {
          row.m_account = Beam::DirectoryEntry::make_account(column);
        }).
      add_column("timestamp", &AdministrationMessageIndex::m_timestamp).
      set_primary_key("id");
    return ROW;
  }

  /** Returns a row representing a Message::Body. */
  inline const auto& get_message_body_row() {
    static const auto ROW = Viper::Row<Message::Body>().
      add_column(
        "content_type", Viper::varchar(100), &Message::Body::m_content_type).
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
  inline const auto& get_indexed_message_body_row() {
    static const auto ROW = Viper::Row<IndexedMessageBody>().
      add_column("id", &IndexedMessageBody::m_id).
      extend(get_message_body_row(), &IndexedMessageBody::m_body).
      add_index("id_index", "id");
    return ROW;
  }

  /** Represents an index to an administration message. */
  struct AccountModificationRequestMessageIndex {

    /** The id of the request the message belongs to. */
    int m_request_id;

    /** The id of the message. */
    int m_message_id;
  };

  /** Returns a row representing a message body. */
  inline const auto& get_account_modification_request_message_index_row() {
    static const auto ROW =
      Viper::Row<AccountModificationRequestMessageIndex>().
        add_column(
          "request_id", &AccountModificationRequestMessageIndex::m_request_id).
        add_column(
          "message_id", &AccountModificationRequestMessageIndex::m_message_id).
        add_index("id_index", "request_id");
    return ROW;
  }
}

#endif
