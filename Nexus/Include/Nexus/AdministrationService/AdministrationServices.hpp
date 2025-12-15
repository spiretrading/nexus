#ifndef NEXUS_ADMINISTRATION_SERVICES_HPP
#define NEXUS_ADMINISTRATION_SERVICES_HPP
#include <string>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {

  /** Standard name for the administration service. */
  inline const auto ADMINISTRATION_SERVICE_NAME =
    std::string("administration_service");

  BEAM_DEFINE_SERVICES(administration_services,

    /**
     * Loads the list of accounts that match a set of roles.
     * @param roles The roles to match.
     * @return The list of directory entries of accounts that match the
     *         specified roles.
     */
    (LoadAccountsByRolesService,
      "Nexus.AdministrationServices.LoadAccountsByRolesService",
      std::vector<Beam::DirectoryEntry>, (AccountRoles, roles)),

    /**
     * Loads the DirectoryEntry containing all administrators.
     * @return The DirectoryEntry containing all administrators.
     */
    (LoadAdministratorsRootEntryService,
      "Nexus.AdministrationServices.LoadAdministratorsRootEntryService",
      Beam::DirectoryEntry),

    /**
     * Loads the DirectoryEntry containing all service accounts.
     * @return DirectoryEntry containing all service accounts.
     */
    (LoadServicesRootEntryService,
      "Nexus.AdministrationServices.LoadServicesRootEntryService",
      Beam::DirectoryEntry),

    /**
     * Loads the DirectoryEntry containing all trading groups.
     * @return The DirectoryEntry containing all trading groups.
     */
    (LoadTradingGroupsRootEntryService,
      "Nexus.AdministrationServices.LoadTradingGroupsRootEntryService",
      Beam::DirectoryEntry),

    /**
     * Tests if an account is an administrator.
     * @param account The account to test.
     * @return <code>true</code> iff the <i>account</i> is an administrator.
     */
    (CheckAdministratorService,
      "Nexus.AdministrationServices.CheckAdministratorService", bool,
      (Beam::DirectoryEntry, account)),

    /**
     * Returns an accounts roles.
     * @param account The account to lookup.
     * @return The roles associated with the <i>account</i>.
     */
    (LoadAccountRolesService,
      "Nexus.AdministrationServices.LoadAccountRolesService", AccountRoles,
      (Beam::DirectoryEntry, account)),

    /**
     * Returns the roles one account has over another.
     * @param parent The account whose roles are to be loaded.
     * @param child The account being supervised.
     * @return The roles that the <i>parent</i> account has over the
     *         <i>child</i> account.
     */
    (LoadSupervisedAccountRolesService,
      "Nexus.AdministrationServices.LoadSupervisedAccountRolesService",
      AccountRoles, (Beam::DirectoryEntry, parent),
      (Beam::DirectoryEntry, child)),

    /**
     * Loads the DirectoryEntry representing an account's trading group.
     * @param account The account whose trading group is to be loaded.
     * @return The directory of the <i>account</i>'s trading group.
     */
    (LoadParentTradingGroupService,
      "Nexus.AdministrationServices.LoadParentTradingGroupService",
      Beam::DirectoryEntry, (Beam::DirectoryEntry, account)),

    /**
     * Loads an account's identity.
     * @param account The account of the identity to load.
     * @return The identity of the <i>account</i>.
     */
    (LoadAccountIdentityService,
      "Nexus.AdministrationServices.LoadAccountIdentityService",
      AccountIdentity, (Beam::DirectoryEntry, account)),

    /**
     * Sets an account's identity.
     * @param account The account of the identity to set.
     * @param identity The identity to set.
     */
    (StoreAccountIdentityService,
      "Nexus.AdministrationServices.StoreAccountIdentityService", void,
      (Beam::DirectoryEntry, account), (AccountIdentity, identity)),

    /**
     * Loads a TradingGroup from its DirectoryEntry.
     * @param directory The DirectoryEntry of the TradingGroup to load.
     * @return The TradingGroup represented by the specified <i>directory</i>.
     */
    (LoadTradingGroupService,
      "Nexus.AdministrationServices.LoadTradingGroupService", TradingGroup,
      (Beam::DirectoryEntry, directory)),

    /**
     * Loads the list of system administrators.
     * @return The list of system administrators.
     */
    (LoadAdministratorsService,
      "Nexus.AdministrationServices.LoadAdministratorsService",
      std::vector<Beam::DirectoryEntry>),

    /**
     * Loads the list of accounts providing system services.
     * @return The list of system service providers.
     */
    (LoadServicesService, "Nexus.AdministrationServices.LoadServicesService",
      std::vector<Beam::DirectoryEntry>),

    /**
     * Loads the database of available entitlements.
     * @return The database of available entitlements.
     */
    (LoadEntitlementsService,
      "Nexus.AdministrationServices.LoadEntitlementsService",
      EntitlementDatabase),

    /**
     * Loads the entitlements granted to an account.
     * @param account The account to load the entitlements for.
     * @return The list of entitlements granted to the <i>account</i>.
     */
    (LoadAccountEntitlementsService,
      "Nexus.AdministrationServices.LoadAccountEntitlementsService",
      std::vector<Beam::DirectoryEntry>, (Beam::DirectoryEntry, account)),

    /**
     * Sets the entitlements granted to an account.
     * @param account The account to set the entitlements for.
     * @param entitlements The entitlements to grant to the <i>account</i>.
     */
    (StoreEntitlementsService,
      "Nexus.AdministrationServices.StoreEntitlementsService", void,
      (Beam::DirectoryEntry, account),
      (std::vector<Beam::DirectoryEntry>, entitlements)),

    /**
     * Monitors an account's RiskParameters.
     * @param account The account to monitor.
     * @return The <i>account</i>'s RiskParameters.
     */
    (MonitorRiskParametersService,
      "Nexus.AdministrationServices.MonitorRiskParametersService",
      RiskParameters, (Beam::DirectoryEntry, account)),

    /**
     * Sets an account's RiskParameters.
     * @param account The account to set the RiskParameters for.
     * @param riskParameters The RiskParameters to assign to the <i>account</i>.
     */
    (StoreRiskParametersService,
      "Nexus.AdministrationServices.StoreRiskParametersService", void,
      (Beam::DirectoryEntry, account), (RiskParameters, risk_parameters)),

    /**
     * Monitors an account's RiskState.
     * @param account The account to monitor.
     * @return The <i>account</i>'s current RiskState.
     */
    (MonitorRiskStateService,
      "Nexus.AdministrationServices.MonitorRiskStateService", RiskState,
      (Beam::DirectoryEntry, account)),

    /**
     * Sets an account's RiskState.
     * @param account The account to set the RiskState for.
     * @param riskState The RiskState to assign to the <i>account</i>.
     */
    (StoreRiskStateService,
      "Nexus.AdministrationServices.StoreRiskStateService", void,
      (Beam::DirectoryEntry, account), (RiskState, risk_state)),

    /**
     * Loads the DirectoryEntries of TradingGroups managed by an account.
     * @param account The account whose managed TradingGroups are to be loaded.
     * @return The list of TradingGroups managed by the <i>account</i>.
     */
    (LoadManagedTradingGroupsService,
      "Nexus.AdministrationServices.LoadManagedTradingGroupsService",
      std::vector<Beam::DirectoryEntry>, (Beam::DirectoryEntry, account)),

    /**
     * Loads an account modification request.
     * @param id The id of the request to load.
     * @return The request with the specified <i>id</i>.
     */
    (LoadAccountModificationRequestService,
      "Nexus.AdministrationServices.LoadAccountModificationRequestService",
      AccountModificationRequest, (AccountModificationRequest::Id, id)),

    /**
     * Given an account, loads the ids of requests to modify that account.
     * @param account The account whose requests are to be loaded.
     * @param start_id The id of the first request to load (exclusive) or -1 to
     *        start with the most recent request.
     * @param max_count The maximum number of ids to load.
     * @return The list of account modification requests.
     */
    (LoadAccountModificationRequestIdsService,
      "Nexus.AdministrationServices.LoadAccountModificationRequestIdsService",
      std::vector<AccountModificationRequest::Id>,
      (Beam::DirectoryEntry, account),
      (AccountModificationRequest::Id, start_id), (int, max_count)),

    /**
     * Given an account, loads the ids of requests that the account is
     * authorized to manage.
     * @param account The account managing the modification requests.
     * @param start_id The id of the first request to load (exclusive) or -1 to
     *        start with the most recent request.
     * @param max_count The maximum number of ids to load.
     * @return The list of account modification requests.
     */
    (LoadManagedAccountModificationRequestIdsService,
      "Nexus.AdministrationServices.LoadManagedAccountModificationRequestIdsService",
      std::vector<AccountModificationRequest::Id>,
      (Beam::DirectoryEntry, account),
      (AccountModificationRequest::Id, start_id), (int, max_count)),

    /**
     * Loads an entitlement modification.
     * @param id The id of the modification to load.
     * @return The entitlement modification with the specified <i>id</i>.
     */
    (LoadEntitlementModificationService,
      "Nexus.AdministrationServices.LoadEntitlementModificationService",
      EntitlementModification, (AccountModificationRequest::Id, id)),

    /**
     * Submits a request to modify an account's entitlements.
     * @param account The account to modify.
     * @param submission_account The account submitting the request.
     * @param modification The modification to apply.
     * @param comment The comment to associate with the request.
     * @return An object representing the request.
     */
    (SubmitEntitlementModificationRequestService,
      "Nexus.AdministrationServices.SubmitEntitlementModificationRequestService",
      AccountModificationRequest, (Beam::DirectoryEntry, account),
      (EntitlementModification, modification), (Message, comment)),

    /**
     * Loads a risk modification.
     * @param id The id of the modification to load.
     * @return The risk modification with the specified <i>id</i>.
     */
    (LoadRiskModificationService,
      "Nexus.AdministrationServices.LoadRiskModificationService",
      RiskModification, (AccountModificationRequest::Id, id)),

    /**
     * Submits a request to modify an account's risk parameters.
     * @param account The account to modify.
     * @param modification The modification to apply.
     * @param comment The comment to associate with the request.
     * @return An object representing the request.
     */
    (SubmitRiskModificationRequestService,
      "Nexus.AdministrationServices.SubmitRiskModificationRequestService",
      AccountModificationRequest, (Beam::DirectoryEntry, account),
      (RiskModification, modification), (Message, comment)),

    /**
     * Loads the status of an account modification request.
     * @param id The id of the request.
     * @return The update representing the current status of the request.
     */
    (LoadAccountModificationRequestStatusService,
      "Nexus.AdministrationServices.LoadAccountModificationRequestStatusService",
      AccountModificationRequest::Update, (AccountModificationRequest::Id, id)),

    /**
     * Approves an account modification request.
     * @param id The id of the request.
     * @param comment The comment to associate with the approval.
     * @return An object representing the update.
     */
    (ApproveAccountModificationRequestService,
      "Nexus.AdministrationServices.ApproveAccountModificationRequestService",
      AccountModificationRequest::Update, (AccountModificationRequest::Id, id),
      (Message, comment)),

    /**
     * Rejects an account modification request.
     * @param id The id of the request.
     * @param comment The comment to associate with the rejection.
     * @return An object representing the update.
     */
    (RejectAccountModificationRequestService,
      "Nexus.AdministrationServices.RejectAccountModificationRequestService",
      AccountModificationRequest::Update, (AccountModificationRequest::Id, id),
      (Message, comment)),

    /**
     * Loads a message.
     * @param id The id of the message to load.
     * @return The message with the specified <i>id</i>.
     */
    (LoadMessageService, "Nexus.AdministrationServices.LoadMessageService",
      Message, (Message::Id, id)),

    /**
     * Loads the list of messages associated with an account modification.
     * @param id The id of the request.
     * @return The list of message ids associated with the request.
     */
    (LoadMessageIdsService,
      "Nexus.AdministrationServices.LoadMessageIdsService",
      std::vector<Message::Id>, (AccountModificationRequest::Id, id)),

    /**
     * Appends a message to an account modification request.
     * @param id The id of the request to send the message to.
     * @param message The message to append.
     * @return The appended message.
     */
    (SendAccountModificationRequestMessageService,
      "Nexus.AdministrationServices.SendAccountModificationRequestMessageService",
      Message, (AccountModificationRequest::Id, id), (Message, message)));

  BEAM_DEFINE_MESSAGES(administration_messages,

    /**
     * Indicates a change in an account's RiskParameters.
     * @param account The account affected.
     * @param risk_parameters The updated parameters for the <i>account</i>.
     */
    (RiskParametersMessage, "Nexus.AdministrationService.RiskParametersMessage",
      (Beam::DirectoryEntry, account),
      (RiskParameters, risk_parameters)),

    /**
     * Indicates a change in an account's RiskState.
     * @param account The account affected.
     * @param risk_state The <i>account</i>'s current RiskState.
     */
    (RiskStateMessage, "Nexus.AdministrationService.RiskStateMessage",
      (Beam::DirectoryEntry, account), (RiskState, risk_state)));
}

#endif
