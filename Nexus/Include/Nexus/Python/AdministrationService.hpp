#ifndef NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#define NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"

namespace Nexus::Python {

  /**
   * Exports the AccountIdentity struct.
   * @param module The module to export to.
   */
  void export_account_identity(pybind11::module& module);

  /**
   * Exports the AccountModificationRequest class.
   * @param module The module to export to.
   */
  void export_account_modification_request(pybind11::module& module);

  /**
   * Exports the AccountRoles enum.
   * @param module The module to export to.
   */
  void export_account_roles(pybind11::module& module);

  /**
   * Exports an AdministrationClient class.
   * @param <C> The type of AdministrationClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported AdministrationClient.
   */
  template<IsAdministrationClient C>
  auto export_administration_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C, std::shared_ptr<C>>(module, name.data()).
      def("load_accounts_by_roles", &C::load_accounts_by_roles).
      def("load_administrators_root_entry", &C::load_administrators_root_entry).
      def("load_services_root_entry", &C::load_services_root_entry).
      def("load_trading_groups_root_entry", &C::load_trading_groups_root_entry).
      def("check_administrator", &C::check_administrator).
      def("load_account_roles", static_cast<AccountRoles (C::*)(
        const Beam::DirectoryEntry&)>(&C::load_account_roles)).
      def("load_account_roles", static_cast<AccountRoles (C::*)(
        const Beam::DirectoryEntry&,
        const Beam::DirectoryEntry&)>(&C::load_account_roles)).
      def("load_parent_trading_group", &C::load_parent_trading_group).
      def("load_identity", &C::load_identity).
      def("store", static_cast<void (C::*)(
        const Beam::DirectoryEntry&, const AccountIdentity&)>(
          &C::store)).
      def("load_trading_group", &C::load_trading_group).
      def("load_managed_trading_groups", &C::load_managed_trading_groups).
      def("load_administrators", &C::load_administrators).
      def("load_services", &C::load_services).
      def("load_entitlements",
        static_cast<std::vector<Beam::DirectoryEntry> (C::*)(
          const Beam::DirectoryEntry&)>(&C::load_entitlements)).
      def("store_entitlements", &C::store_entitlements).
      def("get_risk_parameters_publisher", &C::get_risk_parameters_publisher,
        pybind11::return_value_policy::reference_internal).
      def("store", static_cast<void (C::*)(
        const Beam::DirectoryEntry&, const RiskParameters&)>(
          &C::store)).
      def("get_risk_state_publisher", &C::get_risk_state_publisher,
        pybind11::return_value_policy::reference_internal).
      def("store", static_cast<void (C::*)(
        const Beam::DirectoryEntry&, const RiskState&)>(
          &C::store)).
      def("load_account_modification_request",
        &C::load_account_modification_request).
      def("load_account_modification_request_ids",
        &C::load_account_modification_request_ids).
      def("load_managed_account_modification_request_ids",
        &C::load_managed_account_modification_request_ids).
      def("load_entitlement_modification", &C::load_entitlement_modification).
      def("submit", static_cast<AccountModificationRequest (C::*)(
        const Beam::DirectoryEntry&,
        const EntitlementModification&, const Message&)>(&C::submit)).
      def("load_risk_modification", &C::load_risk_modification).
      def("submit", static_cast<AccountModificationRequest (C::*)(
        const Beam::DirectoryEntry&, const RiskModification&,
        const Message&)>(&C::submit)).
      def("load_account_modification_request_status",
        &C::load_account_modification_request_status).
      def("approve_account_modification_request",
        &C::approve_account_modification_request).
      def("reject_account_modification_request",
        &C::reject_account_modification_request).
      def("load_message", &C::load_message).
      def("load_message_ids", &C::load_message_ids).
      def("send_account_modification_request_message",
        &C::send_account_modification_request_message).
      def("close", &C::close);
    return client;
  }

  /**
   * Exports the AdministrationDataStore class.
   * @param <D> The type of AdministrationDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported AdministrationDataStore.
   */
  template<IsAdministrationDataStore D>
  auto export_administration_data_store(
      pybind11::module& module, std::string_view name) {
    auto data_store =
      pybind11::class_<D, std::shared_ptr<D>>(module, name.data()).
        def("load_all_account_identities", &D::load_all_account_identities).
        def("load_identity", &D::load_identity).
        def("store", static_cast<void (D::*)(
          const Beam::DirectoryEntry&, const AccountIdentity&)>(
            &D::store)).
        def("load_all_risk_parameters", &D::load_all_risk_parameters).
        def("load_risk_parameters", &D::load_risk_parameters).
        def("store", static_cast<void (D::*)(
          const Beam::DirectoryEntry&, const RiskParameters&)>(
            &D::store)).
        def("load_all_risk_states", &D::load_all_risk_states).
        def("load_risk_state", &D::load_risk_state).
        def("store", static_cast<void (D::*)(
          const Beam::DirectoryEntry&, const RiskState&)>(
            &D::store)).
        def("load_account_modification_request",
          &D::load_account_modification_request).
        def("load_account_modification_request_ids",
          static_cast<std::vector<AccountModificationRequest::Id> (D::*)(
            const Beam::DirectoryEntry&,
            AccountModificationRequest::Id, int)>(
              &D::load_account_modification_request_ids)).
        def("load_account_modification_request_ids",
          static_cast<std::vector<AccountModificationRequest::Id> (D::*)(
            AccountModificationRequest::Id, int)>(
              &D::load_account_modification_request_ids)).
        def("load_entitlement_modification", &D::load_entitlement_modification).
        def("store", static_cast<void (D::*)(
          const AccountModificationRequest&, const EntitlementModification&)>(
            &D::store)).
        def("load_risk_modification", &D::load_risk_modification).
        def("store", static_cast<void (D::*)(
          const AccountModificationRequest&, const RiskModification&)>(
            &D::store)).
        def("store", static_cast<void (D::*)(
          AccountModificationRequest::Id, const Message&)>(&D::store)).
        def("load_account_modification_request_status",
          &D::load_account_modification_request_status).
        def("store", static_cast<void (D::*)(AccountModificationRequest::Id,
          const AccountModificationRequest::Update&)>(&D::store)).
        def("load_last_message_id", &D::load_last_message_id).
        def("load_message", &D::load_message).
        def("load_message_ids", &D::load_message_ids).
        def("with_transaction", &D::with_transaction).
        def("close", &D::close);
    return data_store;
  }

  /**
   * Exports the AdministrationDataStoreException class.
   * @param module The module to export to.
   */
  void export_administration_data_store_exception(pybind11::module& module);

  /**
   * Exports all sdministration service functionality.
   * @param module The module to export to.
   */
  void export_administration_service(pybind11::module& module);

  /**
   * Exports the AdministrationServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_administration_service_test_environment(pybind11::module& module);

  /**
   * Exports the CachedAdministrationDataStore class.
   * @param module The module to export to.
   */
  void export_cached_administration_data_store(pybind11::module& module);

  /**
   * Exports the EntitlementModificationRequest class.
   * @param module The module to export to.
   */
  void export_entitlement_modification(pybind11::module& module);

  /**
   * Exports the IndexedAccountIdentity struct.
   * @param module The module to export to.
   */
  void export_indexed_account_identity(pybind11::module& module);

  /**
   * Exports the IndexedRiskParameters struct.
   * @param module The module to export to.
   */
  void export_indexed_risk_parameters(pybind11::module& module);

  /**
   * Exports the IndexedRiskState struct.
   * @param module The module to export to.
   */
  void export_indexed_risk_state(pybind11::module& module);

  /**
   * Exports the LocalAdministrationDataStore class.
   * @param module The module to export to.
   */
  void export_local_administration_data_store(pybind11::module& module);

  /**
   * Exports the Message class.
   * @param module The module to export to.
   */
  void export_message(pybind11::module& module);

  /**
   * Exports the MySqlAdministrationDataStore class.
   * @param module The module to export to.
   */
  void export_mysql_administration_data_store(pybind11::module& module);

  /**
   * Exports the RiskModificationRequest class.
   * @param module The module to export to.
   */
  void export_risk_modification(pybind11::module& module);

  /**
   * Exports the SqliteAdministrationDataStore class.
   * @param module The module to export to.
   */
  void export_sqlite_administration_data_store(pybind11::module& module);

  /**
   * Exports the TradingGroup class.
   * @param module The module to export to.
   */
  void export_trading_group(pybind11::module& module);
}

#endif
