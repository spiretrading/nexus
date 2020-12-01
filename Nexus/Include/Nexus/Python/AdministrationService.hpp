#ifndef NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#define NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported AdministrationClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<
    AdministrationService::AdministrationClientBox>&
      GetExportedAdministrationClientBox();

  /**
   * Exports the AccountIdentity struct.
   * @param module The module to export to.
   */
  void ExportAccountIdentity(pybind11::module& module);

  /**
   * Exports the AccountModificationRequest class.
   * @param module The module to export to.
   */
  void ExportAccountModificationRequest(pybind11::module& module);

  /**
   * Exports the AdministrationService namespace.
   * @param module The module to export to.
   */
  void ExportAdministrationService(pybind11::module& module);

  /**
   * Exports the AdministrationServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportAdministrationServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the ApplicationAdministrationClient class.
   * @param module The module to export to.
   */
  void ExportApplicationAdministrationClient(pybind11::module& module);

  /**
   * Exports the EntitlementModificationRequest class.
   * @param module The module to export to.
   */
  void ExportEntitlementModification(pybind11::module& module);

  /**
   * Exports the Message class.
   * @param module The module to export to.
   */
  void ExportMessage(pybind11::module& module);

  //! Exports the TradingGroup class.
  /**
   * Exports the TradingGroup class.
   * @param module The module to export to.
   */
  void ExportTradingGroup(pybind11::module& module);

  /**
   * Exports an AdministrationClient class.
   * @param <Client> The type of AdministrationClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported AdministrationClient.
   */
  template<typename Client>
  auto ExportAdministrationClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("load_accounts_by_roles", &Client::LoadAccountsByRoles).
      def("load_administrators_root_entry",
        &Client::LoadAdministratorsRootEntry).
      def("load_services_root_entry", &Client::LoadServicesRootEntry).
      def("load_trading_groups_root_entry",
        &Client::LoadTradingGroupsRootEntry).
      def("check_administrator", &Client::CheckAdministrator).
      def("load_account_roles", static_cast<
        AdministrationService::AccountRoles (Client::*)(
          const Beam::ServiceLocator::DirectoryEntry&)>(
            &Client::LoadAccountRoles)).
      def("load_account_roles", static_cast<
        AdministrationService::AccountRoles (Client::*)(
          const Beam::ServiceLocator::DirectoryEntry&,
          const Beam::ServiceLocator::DirectoryEntry&)>(
            &Client::LoadAccountRoles)).
      def("load_parent_trading_group", &Client::LoadParentTradingGroup).
      def("load_identity", &Client::LoadIdentity).
      def("store_identity", &Client::StoreIdentity).
      def("load_trading_group", &Client::LoadTradingGroup).
      def("load_managed_trading_groups", &Client::LoadManagedTradingGroups).
      def("load_administrators", &Client::LoadAdministrators).
      def("load_services", &Client::LoadServices).
      def("load_entitlements", static_cast<
        MarketDataService::EntitlementDatabase (Client::*)()>(
          &Client::LoadEntitlements)).
      def("load_entitlements", static_cast<
        std::vector<Beam::ServiceLocator::DirectoryEntry> (Client::*)(
          const Beam::ServiceLocator::DirectoryEntry&)>(
            &Client::LoadEntitlements)).
      def("store_entitlements", &Client::StoreEntitlements).
      def("get_risk_parameters_publisher", &Client::GetRiskParametersPublisher,
        pybind11::return_value_policy::reference_internal).
      def("store_risk_parameters", &Client::StoreRiskParameters).
      def("get_risk_state_publisher", &Client::GetRiskStatePublisher,
        pybind11::return_value_policy::reference_internal).
      def("store_risk_state", &Client::StoreRiskState).
      def("load_account_modification_request",
        &Client::LoadAccountModificationRequest).
      def("load_account_modification_request_ids",
        &Client::LoadAccountModificationRequestIds).
      def("load_managed_account_modification_request_ids",
        &Client::LoadManagedAccountModificationRequestIds).
      def("load_entitlement_modification",
        &Client::LoadEntitlementModification).
      def("submit_account_modification_request", static_cast<
        AdministrationService::AccountModificationRequest (Client::*)(
          const Beam::ServiceLocator::DirectoryEntry&,
          const AdministrationService::EntitlementModification&,
          const AdministrationService::Message&)>(
            &Client::SubmitAccountModificationRequest)).
      def("load_risk_modification", &Client::LoadRiskModification).
      def("submit_account_modification_request", static_cast<
        AdministrationService::AccountModificationRequest (Client::*)(
          const Beam::ServiceLocator::DirectoryEntry&,
          const AdministrationService::RiskModification&,
          const AdministrationService::Message&)>(
            &Client::SubmitAccountModificationRequest)).
      def("load_account_modification_request_status",
        &Client::LoadAccountModificationRequestStatus).
      def("approve_account_modification_request",
        &Client::ApproveAccountModificationRequest).
      def("reject_account_modification_request",
        &Client::RejectAccountModificationRequest).
      def("load_message", &Client::LoadMessage).
      def("load_message_ids", &Client::LoadMessageIds).
      def("send_account_modification_request_message",
        &Client::SendAccountModificationRequestMessage).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client,
        AdministrationService::AdministrationClientBox>) {
      pybind11::implicitly_convertible<Client,
        AdministrationService::AdministrationClientBox>();
      GetExportedAdministrationClientBox().def(pybind11::init<
        std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
