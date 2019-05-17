#ifndef NEXUS_ADMINISTRATION_SERVICE_SQL_DEFINITIONS_HPP
#define NEXUS_ADMINISTRATION_SERVICE_SQL_DEFINITIONS_HPP
#include <Beam/Sql/Conversions.hpp>
#include <Viper/Row.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus::AdministrationService {

  //! Returns a row representing an AccountIdentity.
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

  //! Represents an AccountIdentity indexed by account id.
  struct IndexedAccountIdentity {

    //! The account's id.
    unsigned int m_account;

    //! The account's identity.
    AccountIdentity m_identity;

    //! The photo ID path.
    std::string m_photoId;
  };

  //! Returns a row representing an IndexedAccountIdentity.
  inline const auto& GetIndexedAccountIdentityRow() {
    static auto ROW = Viper::Row<IndexedAccountIdentity>().
      add_column("account", &IndexedAccountIdentity::m_account).
      extend(GetAccountIdentityRow(), &IndexedAccountIdentity::m_identity).
      add_column("photo_id", Viper::varchar(256),
        &IndexedAccountIdentity::m_photoId).
      set_primary_key("account");
    return ROW;
  }

  //! Returns a row representing RiskParameters.
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
      add_column("transitition_time",
        &RiskService::RiskParameters::m_transitionTime);
    return ROW;
  }

  //! Represents RiskParameters indexed by account.
  struct IndexedRiskParameters {

    //! The account's id.
    unsigned int m_account;

    //! The RiskParameters.
    RiskService::RiskParameters m_parameters;
  };

  //! Returns a row representing IndexedRiskParameters.
  inline const auto& GetIndexedRiskParametersRow() {
    static auto ROW = Viper::Row<IndexedRiskParameters>().
      add_column("account", Viper::uinteger, &IndexedRiskParameters::m_account).
      extend(GetRiskParametersRow(), &IndexedRiskParameters::m_parameters).
      set_primary_key("account");
    return ROW;
  }
}

#endif
