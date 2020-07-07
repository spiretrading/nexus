#ifndef NEXUS_ADMINISTRATION_SERVICES_HPP
#define NEXUS_ADMINISTRATION_SERVICES_HPP
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {
namespace AdministrationService {
  BEAM_DEFINE_SERVICES(AdministrationServices,
    /*! \interface Nexus::AdministrationServices::LoadAccountsByRolesService
        \brief Loads the list of accounts that match a set of roles.
        \param roles <code>Nexus::AdministrationService::AccountRoles</code>
               The roles to match.
        \return <code>std::vector\<Beam::ServiceLocator::DirectoryEntry\></code>
                The list of directory entries of accounts that match the
                specified roles.
    */
    //! \cond
    (LoadAccountsByRolesService,
      "Nexus.AdministrationServices.LoadAccountsByRolesService",
      std::vector<Beam::ServiceLocator::DirectoryEntry>, AccountRoles, roles),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadAdministratorsRootEntryService
        \brief Loads the DirectoryEntry containing all administrators.
        \param dummy <code>int</code> Unused.
        \return <code>Beam::ServiceLocator::DirectoryEntry</code> The
                DirectoryEntry containing all administrators.
    */
    //! \cond
    (LoadAdministratorsRootEntryService,
      "Nexus.AdministrationServices.LoadAdministratorsRootEntryService",
      Beam::ServiceLocator::DirectoryEntry, int, dummy),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadServicesRootEntryService
        \brief Loads the DirectoryEntry containing all service accounts.
        \param dummy <code>int</code> Unused.
        \return <code>Beam::ServiceLocator::DirectoryEntry</code> The
                DirectoryEntry containing all service accounts.
    */
    //! \cond
    (LoadServicesRootEntryService,
      "Nexus.AdministrationServices.LoadServicesRootEntryService",
      Beam::ServiceLocator::DirectoryEntry, int, dummy),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadTradingGroupsRootEntryService
        \brief Loads the DirectoryEntry containing all trading groups.
        \param dummy <code>int</code> Unused.
        \return <code>Beam::ServiceLocator::DirectoryEntry</code> The
                DirectoryEntry containing all trading groups.
    */
    //! \cond
    (LoadTradingGroupsRootEntryService,
      "Nexus.AdministrationServices.LoadTradingGroupsRootEntryService",
      Beam::ServiceLocator::DirectoryEntry, int, dummy),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::CheckAdministratorService
        \brief Tests if an account is an administrator.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to test.
        \return <code>bool</code><code>true</code> iff the <i>account</i> is an
                administrator.
    */
    //! \cond
    (CheckAdministratorService,
      "Nexus.AdministrationServices.CheckAdministratorService", bool,
      Beam::ServiceLocator::DirectoryEntry, account),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadAccountRolesService
        \brief Returns an accounts roles.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to lookup.
        \return <code>AccountRoles</code>The roles associated with the
                <i>account</i>.
    */
    //! \cond
    (LoadAccountRolesService,
      "Nexus.AdministrationServices.LoadAccountRolesService", AccountRoles,
      Beam::ServiceLocator::DirectoryEntry, account),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadSupervisedAccountRolesService
        \brief Returns the roles one account has over another.
        \param parent <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account whose roles are to be loaded.
        \param child <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account being supervised.
        \return <code>AccountRoles</code> The roles that the <i>parent</i>
                account has over the <i>child</i> account.
    */
    //! \cond
    (LoadSupervisedAccountRolesService,
      "Nexus.AdministrationServices.LoadSupervisedAccountRolesService",
      AccountRoles, Beam::ServiceLocator::DirectoryEntry, parent,
      Beam::ServiceLocator::DirectoryEntry, child),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadAccountTradingGroupEntryService
        \brief Loads an account's trading group Directory.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account whose trading group is to be loaded.
        \return <code>Beam::ServiceLocator::DirectoryEntry</code> The directory
                of the trading <i>account</i>'s group.
    */
    //! \cond
    (LoadAccountTradingGroupEntryService,
      "Nexus.AdministrationServices.LoadAccountTradingGroupEntryService",
      Beam::ServiceLocator::DirectoryEntry,
      Beam::ServiceLocator::DirectoryEntry, account),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadAccountIdentityService
        \brief Loads an account's identity.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account of the identity to load.
        \return <code>AccountIdentity</code> The identity of the <i>account</i>.
    */
    //! \cond
    (LoadAccountIdentityService,
      "Nexus.AdministrationServices.LoadAccountIdentityService",
      AccountIdentity, Beam::ServiceLocator::DirectoryEntry, account),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::StoreAccountIdentityService
        \brief Sets an account's identity.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account of the identity to set.
        \param identity <code>AccountIdentity</code> The identity to set.
    */
    //! \cond
    (StoreAccountIdentityService,
      "Nexus.AdministrationServices.StoreAccountIdentityService", void,
      Beam::ServiceLocator::DirectoryEntry, account, AccountIdentity,
      identity),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadTradingGroupService
        \brief Loads a TradingGroup from its DirectoryEntry.
        \param directory <code>Beam::ServiceLocator::DirectoryEntry</code> The
               DirectoryEntry of the TradingGroup to load.
        \return <code>TradingGroup</code> The TradingGroup represented by the
                specified <i>directory</i>.
    */
    //! \cond
    (LoadTradingGroupService,
      "Nexus.AdministrationServices.LoadTradingGroupService", TradingGroup,
      Beam::ServiceLocator::DirectoryEntry, directory),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadAdministratorsService
        \brief Loads the list of system administrators.
        \param dummy <code>int</code> Unused.
        \return <code>std::vector\<Beam::ServiceLocator::DirectoryEntry\>
                </code> The list of system administrators.
    */
    //! \cond
    (LoadAdministratorsService,
      "Nexus.AdministrationServices.LoadAdministratorsService",
      std::vector<Beam::ServiceLocator::DirectoryEntry>, int, dummy),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadServicesService
        \brief Loads the list of accounts providing system services.
        \param dummy <code>int</code> Unused.
        \return <code>std::vector\<Beam::ServiceLocator::DirectoryEntry\>
                </code> The list of system service providers.
    */
    //! \cond
    (LoadServicesService,
      "Nexus.AdministrationServices.LoadServicesService",
      std::vector<Beam::ServiceLocator::DirectoryEntry>, int, dummy),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadEntitlementsService
        \brief Loads the database of available entitlements.
        \param dummy <code>int</code> Unused.
        \return <code>Nexus::MarketDataService::EntitlementDatabase</code> The
                database of available entitlements.
    */
    //! \cond
    (LoadEntitlementsService,
      "Nexus.AdministrationServices.LoadEntitlementsService",
      MarketDataService::EntitlementDatabase, int, dummy),
    //! \endcond

    /*! \interface Nexus::AdministrationServices::LoadAccountEntitlementsService
        \brief Loads the entitlements granted to an account.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to load the entitlements for.
        \return <code>std::vector\<Beam::ServiceLocator::DirectoryEntry\>
                </code> The list of entitlements granted to the <i>account</i>.
    */
    //! \cond
    (LoadAccountEntitlementsService,
      "Nexus.AdministrationServices.LoadAccountEntitlementsService",
      std::vector<Beam::ServiceLocator::DirectoryEntry>,
      Beam::ServiceLocator::DirectoryEntry, account),

    /*! \interface Nexus::AdministrationServices::StoreEntitlementsService
        \brief Sets the entitlements granted to an account.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to set the entitlements for.
        \param entitlements <code>
               std::vector\<Beam::ServiceLocator::DirectoryEntry\></code>
               The entitlements to grant to the <i>account</i>.
    */
    //! \cond
    (StoreEntitlementsService,
      "Nexus.AdministrationServices.StoreEntitlementsService", void,
      Beam::ServiceLocator::DirectoryEntry, account,
      std::vector<Beam::ServiceLocator::DirectoryEntry>, entitlements),

    /*! \interface Nexus::AdministrationServices::MonitorRiskParametersService
        \brief Monitors an account's RiskParameters.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to monitor.
        \return <code>RiskService::RiskParameters</code> The <i>account</i>'s
                RiskParameters.
    */
    //! \cond
    (MonitorRiskParametersService,
      "Nexus.AdministrationServices.MonitorRiskParametersService",
      RiskService::RiskParameters, Beam::ServiceLocator::DirectoryEntry,
      account),

    /*! \interface Nexus::AdministrationServices::StoreRiskParametersService
        \brief Sets an account's RiskParameters.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to set the RiskParameters for.
        \param riskParameters <code>RiskService::RiskParameters</code> The
               RiskParameters to assign to the <i>account</i>.
    */
    //! \cond
    (StoreRiskParametersService,
      "Nexus.AdministrationServices.StoreRiskParametersService", void,
      Beam::ServiceLocator::DirectoryEntry, account,
      RiskService::RiskParameters, risk_parameters),

    /*! \interface Nexus::AdministrationServices::MonitorRiskStateService
        \brief Monitors an account's RiskState.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to monitor.
        \return <code>RiskService::RiskState</code> The <i>account</i>'s current
                RiskState.
    */
    //! \cond
    (MonitorRiskStateService,
      "Nexus.AdministrationServices.MonitorRiskStateService",
      RiskService::RiskState, Beam::ServiceLocator::DirectoryEntry, account),

    /*! \interface Nexus::AdministrationServices::StoreRiskStateService
        \brief Sets an account's RiskState.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to set the RiskState for.
        \param riskState <code>RiskService::RiskState</code> The RiskState to
               assign to the <i>account</i>.
    */
    //! \cond
    (StoreRiskStateService,
      "Nexus.AdministrationServices.StoreRiskStateService", void,
      Beam::ServiceLocator::DirectoryEntry, account, RiskService::RiskState,
      risk_state),

    /*! \interface Nexus::AdministrationServices::LoadManagedTradingGroupsService
        \brief Loads the DirectoryEntries of TradingGroups managed by an
               account.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code>
               The account whose managed TradingGroups are to be loaded.
        \return <code>std::vector\<Beam::ServiceLocator::DirectoryEntry\>
                </code> The list of TradingGroups managed by the <i>account</i>.
    */
    //! \cond
    (LoadManagedTradingGroupsService,
      "Nexus.AdministrationServices.LoadManagedTradingGroupsService",
      std::vector<Beam::ServiceLocator::DirectoryEntry>,
      Beam::ServiceLocator::DirectoryEntry, account),

    /*! \interface Nexus::AdministrationServices::LoadAccountModificationRequestService
        \brief Loads an account modification request.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the request to load.
        \return <code>Nexus::AdministrationService::AccountModificationRequest</code>
                The request with the specified <i>id</i>.
    */
    //! \cond
    (LoadAccountModificationRequestService,
      "Nexus.AdministrationServices.LoadAccountModificationRequestService",
      AccountModificationRequest, AccountModificationRequest::Id, id),

    /*! \interface Nexus::AdministrationServices::LoadAccountModificationRequestIdsService
        \brief Given an account, loads the ids of requests to modify that
               account.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account whose requests are to be loaded.
        \param start_id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the first request to load (exclusive) or -1 to start
               with the most recent request.
        \param max_count <code>int</code> The maximum number of ids to load.
        \return <code>std::vector\<Nexus::AdministrationService::AccountModificationRequest::Id\></code>
                The list of account modification requests.
    */
    //! \cond
    (LoadAccountModificationRequestIdsService,
      "Nexus.AdministrationServices.LoadAccountModificationRequestIdsService",
      std::vector<AccountModificationRequest::Id>,
      Beam::ServiceLocator::DirectoryEntry, account,
      AccountModificationRequest::Id, start_id, int, max_count),

    /*! \interface Nexus::AdministrationServices::LoadManagedAccountModificationRequestIdsService
        \brief Given an account, loads the ids of requests that the account is
               authorized to manage.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account managing the modification requests.
        \param start_id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the first request to load (exclusive) or -1 to start
               with the most recent request.
        \param max_count <code>int</code> The maximum number of ids to load.
        \return <code>std::vector\<Nexus::AdministrationService::AccountModificationRequest::Id\></code>
                The list of account modification requests.
    */
    //! \cond
    (LoadManagedAccountModificationRequestIdsService,
      "Nexus.AdministrationServices.LoadManagedAccountModificationRequestIdsService",
      std::vector<AccountModificationRequest::Id>,
      Beam::ServiceLocator::DirectoryEntry, account,
      AccountModificationRequest::Id, start_id, int, max_count),

    /*! \interface Nexus::AdministrationServices::LoadEntitlementModificationService
        \brief Loads an entitlement modification.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the modification to load.
        \return <code>Nexus::AdministrationService::EntitlementModification</code>
                The entitlement modification with the specified <i>id</i>.
    */
    //! \cond
    (LoadEntitlementModificationService,
      "Nexus.AdministrationServices.LoadEntitlementModificationService",
      EntitlementModification, AccountModificationRequest::Id, id),

    /*! \interface Nexus::AdministrationServices::SubmitEntitlementModificationRequestService
        \brief Submits a request to modify an account's entitlements.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to modify.
        \param submission_account <code>Beam::ServiceLocator::DirectoryEntry</code>
               The account submitting the request.
        \param modification <code>Nexus::AdministrationService::EntitlementModification</code>
               The modification to apply.
        \param comment <code>Nexus::AdministrationService::Message</code> The
               comment to associate with the request.
        \return <code>Nexus::AdministrationService::AccountModificationRequest</code>
                An object representing the request.
    */
    //! \cond
    (SubmitEntitlementModificationRequestService,
      "Nexus.AdministrationServices.SubmitEntitlementModificationRequestService",
      AccountModificationRequest, Beam::ServiceLocator::DirectoryEntry, account,
      EntitlementModification, modification, Message, comment),

    /*! \interface Nexus::AdministrationServices::LoadRiskModificationService
        \brief Loads a risk modification.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the modification to load.
        \return <code>Nexus::AdministrationService::RiskModification</code>
                The risk modification with the specified <i>id</i>.
    */
    //! \cond
    (LoadRiskModificationService,
      "Nexus.AdministrationServices.LoadRiskModificationService",
      RiskModification, AccountModificationRequest::Id, id),

    /*! \interface Nexus::AdministrationServices::SubmitRiskModificationRequestService
        \brief Submits a request to modify an account's risk parameters.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account to modify.
        \param modification <code>Nexus::AdministrationService::RiskModification</code>
               The modification to apply.
        \param comment <code>Nexus::AdministrationService::Message</code> The
               comment to associate with the request.
        \return <code>Nexus::AdministrationService::AccountModificationRequest</code>
                An object representing the request.
    */
    //! \cond
    (SubmitRiskModificationRequestService,
      "Nexus.AdministrationServices.SubmitRiskModificationRequestService",
      AccountModificationRequest, Beam::ServiceLocator::DirectoryEntry, account,
      RiskModification, modification, Message, comment),

    /*! \interface Nexus::AdministrationServices::LoadAccountModificationRequestStatusService
        \brief Loads the status of an account modification request.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the request.
        \return <code>Nexus::AdministrationService::AccountModificationRequest::Update</code>
                The update representing the current status of the request.
    */
    //! \cond
    (LoadAccountModificationRequestStatusService,
      "Nexus.AdministrationServices.LoadAccountModificationRequestStatusService",
      AccountModificationRequest::Update, AccountModificationRequest::Id, id),

    /*! \interface Nexus::AdministrationServices::ApproveAccountModificationRequestService
        \brief Approves an account modification request.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the request.
        \param comment <code>Nexus::AdministrationService::Message</code> The
               comment to associate with the approval.
        \return <code>Nexus::AdministrationService::AccountModificationRequest::Update</code>
                An object representing the update.
    */
    //! \cond
    (ApproveAccountModificationRequestService,
      "Nexus.AdministrationServices.ApproveAccountModificationRequestService",
      AccountModificationRequest::Update, AccountModificationRequest::Id, id,
      Message, comment),

    /*! \interface Nexus::AdministrationServices::RejectAccountModificationRequestService
        \brief Rejects an account modification request.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the request.
        \param comment <code>Nexus::AdministrationService::Message</code> The
               comment to associate with the rejection.
        \return <code>Nexus::AdministrationService::AccountModificationRequest::Update</code>
                An object representing the update.
    */
    //! \cond
    (RejectAccountModificationRequestService,
      "Nexus.AdministrationServices.RejectAccountModificationRequestService",
      AccountModificationRequest::Update, AccountModificationRequest::Id, id,
      Message, comment),

    /*! \interface Nexus::AdministrationServices::LoadMessageService
        \brief Loads a message.
        \param id <code>Nexus::AdministrationService::Message::Id</code>
               The id of the message to load.
        \return <code>Nexus::AdministrationService::Message</code>
                The message with the specified <i>id</i>.
    */
    //! \cond
    (LoadMessageService, "Nexus.AdministrationServices.LoadMessageService",
      Message, Message::Id, id),

    /*! \interface Nexus::AdministrationServices::LoadMessageIdsService
        \brief Loads the list of messages associated with an account
               modification.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the request.
        \return <code>std::vector\<Nexus::AdministrationServices::Message::Id\></code>
                The list of message ids associated with the request.
    */
    //! \cond
    (LoadMessageIdsService,
      "Nexus.AdministrationServices.LoadMessageIdsService",
      std::vector<Message::Id>, AccountModificationRequest::Id, id),

    /*! \interface Nexus::AdministrationServices::SendAccountModificationRequestMessageService
        \brief Appends a message to an account modification request.
        \param id <code>Nexus::AdministrationService::AccountModificationRequest::Id</code>
               The id of the request to send the message to.
        \param message <code>Nexus::AdministrationService::Message</code> The
               message to append.
        \return <code>Nexus::AdministrationServices::Message</code>
                The appended message.
    */
    //! \cond
    (SendAccountModificationRequestMessageService,
      "Nexus.AdministrationServices.SendAccountModificationRequestMessageService",
      Message, AccountModificationRequest::Id, id, Message, message));
    //! \endcond

  BEAM_DEFINE_MESSAGES(AdministrationMessages,

    /*! \interface Nexus::AdministrationService::RiskParametersMessage
        \brief Indicates a change in an account's RiskParameters.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account affected.
        \param risk_parameters <code>RiskService::RiskParameters</code> The
               updated parameters for the <i>account</i>.
    */
    //! \cond
    (RiskParametersMessage, "Nexus.AdministrationService.RiskParametersMessage",
      Beam::ServiceLocator::DirectoryEntry, account,
      RiskService::RiskParameters, risk_parameters),

    /*! \interface Nexus::AdministrationService::RiskStateMessage
        \brief Indicates a change in an account's RiskState.
        \param account <code>Beam::ServiceLocator::DirectoryEntry</code> The
               account affected.
        \param risk_state <code>RiskService::RiskState</code> The
               <i>account</i>'s current RiskState.
    */
    //! \cond
    (RiskStateMessage, "Nexus.AdministrationService.RiskStateMessage",
      Beam::ServiceLocator::DirectoryEntry, account, RiskService::RiskState,
      risk_state));
    //! \endcond
}
}

#endif
