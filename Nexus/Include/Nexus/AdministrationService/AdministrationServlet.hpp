#ifndef NEXUS_ADMINISTRATION_SERVLET_HPP
#define NEXUS_ADMINISTRATION_SERVLET_HPP
#include <atomic>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/AdministrationServices.hpp"
#include "Nexus/AdministrationService/AdministrationSession.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus::AdministrationService {

  /**
   * Provides management and administration services for Nexus accounts.
   * @param <C> The container instantiating this servlet.
   * @param <S> The type of ServiceLocatorClient used to manage accounts.
   * @param <D> The type of AdministrationDataStore to use.
   */
  template<typename C, typename S, typename D>
  class AdministrationServlet : private boost::noncopyable {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of ServiceLocatorClient used to manage accounts. */
      using ServiceLocatorClient = Beam::GetTryDereferenceType<S>;

      /** The type of AdministrationDataStore used. */
      using AdministrationDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs an AdministrationServlet.
       * @param serviceLocatorClient Initializes the ServiceLocatorClient.
       * @param entitlements The list of available entitlements.
       * @param dataStore Initializes the AdministrationDataStore.
       */
      template<typename SF, typename DF>
      AdministrationServlet(SF&& serviceLocatorClient,
        MarketDataService::EntitlementDatabase entitlements, DF&& dataStore);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      struct RiskStateEntry {
        RiskService::RiskState m_riskState;
        std::vector<ServiceProtocolClient*> m_subscribers;
      };
      using RiskStateEntries = std::unordered_map<
        Beam::ServiceLocator::DirectoryEntry, RiskStateEntry>;
      using SyncRiskStateEntries = Beam::Threading::Sync<RiskStateEntries>;
      using RiskParameterSubscribers = std::vector<ServiceProtocolClient*>;
      using SyncRiskParameterSubscribers =
        Beam::Threading::Sync<RiskParameterSubscribers>;
      using AccountToRiskSubscribers = std::unordered_map<
        Beam::ServiceLocator::DirectoryEntry, SyncRiskParameterSubscribers>;
      using SyncAccountToSubscribers =
        Beam::Threading::Sync<AccountToRiskSubscribers>;
      Beam::GetOptionalLocalPtr<S> m_serviceLocatorClient;
      MarketDataService::EntitlementDatabase m_entitlements;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      Beam::ServiceLocator::DirectoryEntry m_administratorsRoot;
      Beam::ServiceLocator::DirectoryEntry m_servicesRoot;
      Beam::ServiceLocator::DirectoryEntry m_tradingGroupsRoot;
      SyncAccountToSubscribers m_riskParametersSubscribers;
      SyncRiskStateEntries m_riskStateEntries;
      std::atomic_int m_nextModificationRequestId;
      std::atomic_int m_nextMessageId;
      Beam::IO::OpenState m_openState;

      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);
      bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);
      bool CheckReadPermission(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(
        const Beam::ServiceLocator::DirectoryEntry& account);
      TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory);
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void GrantEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& adminAccount,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements);
      void UpdateRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& parameters);
      void EnsureModificationReadPermission(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id id);
      AccountModificationRequest MakeModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& sessionAccount,
        const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountRoles& roles, AccountModificationRequest::Type type);
      void StoreModificationRequest(const AccountModificationRequest& request,
        const Message& comment, const AccountRoles& roles);
      std::vector<Beam::ServiceLocator::DirectoryEntry> OnLoadAccountsByRoles(
        ServiceProtocolClient& client, AccountRoles roles);
      Beam::ServiceLocator::DirectoryEntry OnLoadAdministratorsRootEntry(
        ServiceProtocolClient& client);
      Beam::ServiceLocator::DirectoryEntry OnLoadServicesRootEntry(
        ServiceProtocolClient& client);
      Beam::ServiceLocator::DirectoryEntry OnLoadTradingGroupsRootEntry(
        ServiceProtocolClient& client);
      bool OnCheckAdministratorRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles OnLoadAccountRolesRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles OnLoadSupervisedAccountRolesRequest(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);
      Beam::ServiceLocator::DirectoryEntry OnLoadParentTradingGroupRequest(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountIdentity OnLoadAccountIdentityRequest(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void OnStoreAccountIdentityRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);
      TradingGroup OnLoadTradingGroupRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directory);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        OnLoadAdministratorsRequest(ServiceProtocolClient& client);
      std::vector<Beam::ServiceLocator::DirectoryEntry> OnLoadServicesRequest(
        ServiceProtocolClient& client);
      MarketDataService::EntitlementDatabase OnLoadEntitlementsRequest(
        ServiceProtocolClient& client);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        OnLoadAccountEntitlementsRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void OnStoreEntitlementsRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        entitlements);
      RiskService::RiskParameters OnMonitorRiskParametersRequest(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void OnStoreRiskParametersRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& parameters);
      RiskService::RiskState OnMonitorRiskStateRequest(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void OnStoreRiskStateRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, StoreRiskStateService>& request,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        OnLoadManagedTradingGroupsRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountModificationRequest OnLoadAccountModificationRequest(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      std::vector<AccountModificationRequest::Id>
        OnLoadAccountModificationRequestIds(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount);
      std::vector<AccountModificationRequest::Id>
        OnLoadManagedAccountModificationRequestIds(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount);
      EntitlementModification OnLoadEntitlementModification(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      AccountModificationRequest OnSubmitEntitlementModificationRequest(
        ServiceProtocolClient& client,
        Beam::ServiceLocator::DirectoryEntry account,
        const EntitlementModification& modification, Message comment);
      RiskModification OnLoadRiskModification(ServiceProtocolClient& client,
        AccountModificationRequest::Id id);
      AccountModificationRequest OnSubmitRiskModificationRequest(
        ServiceProtocolClient& client,
        Beam::ServiceLocator::DirectoryEntry account,
        const RiskModification& modification, Message comment);
      AccountModificationRequest::Update OnLoadAccountModificationRequestStatus(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      AccountModificationRequest::Update OnApproveAccountModificationRequest(
        ServiceProtocolClient& client, AccountModificationRequest::Id id,
        Message comment);
      AccountModificationRequest::Update OnRejectAccountModificationRequest(
        ServiceProtocolClient& client, AccountModificationRequest::Id id,
        Message comment);
      Message OnLoadMessage(ServiceProtocolClient& client, Message::Id id);
      std::vector<Message::Id> OnLoadMessageIds(ServiceProtocolClient& client,
        AccountModificationRequest::Id id);
      Message OnSendAccountModificationRequestMessage(
        ServiceProtocolClient& client, AccountModificationRequest::Id id,
        Message message);
  };

  template<typename S, typename D>
  struct MetaAdministrationServlet {
    using Session = AdministrationSession;
    template<typename C>
    struct apply {
      using type = AdministrationServlet<C, S, D>;
    };
  };

  template<typename C, typename S, typename D>
  template<typename SF, typename DF>
  AdministrationServlet<C, S, D>::AdministrationServlet(
    SF&& serviceLocatorClient,
    MarketDataService::EntitlementDatabase entitlements, DF&& dataStore)
    : m_serviceLocatorClient(std::forward<SF>(serviceLocatorClient)),
      m_entitlements(std::move(entitlements)),
      m_dataStore(std::forward<DF>(dataStore)) {
    try {
      auto requestIds = m_dataStore->LoadAccountModificationRequestIds(-1, 1);
      if(requestIds.empty()) {
        m_nextModificationRequestId = 0;
      } else {
        m_nextModificationRequestId = requestIds.back();
      }
      m_nextMessageId = m_dataStore->LoadLastMessageId();
      m_administratorsRoot = Beam::ServiceLocator::LoadOrCreateDirectory(
        *m_serviceLocatorClient, "administrators",
        Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
      m_servicesRoot = Beam::ServiceLocator::LoadOrCreateDirectory(
        *m_serviceLocatorClient, "services",
        Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
      m_tradingGroupsRoot = Beam::ServiceLocator::LoadOrCreateDirectory(
        *m_serviceLocatorClient, "trading_groups",
        Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterAdministrationServices(Store(slots));
    RegisterAdministrationMessages(Store(slots));
    LoadAccountsByRolesService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountsByRoles, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAdministratorsRootEntryService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAdministratorsRootEntry, this,
      std::placeholders::_1));
    LoadServicesRootEntryService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadServicesRootEntry, this,
      std::placeholders::_1));
    LoadTradingGroupsRootEntryService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadTradingGroupsRootEntry, this,
      std::placeholders::_1));
    CheckAdministratorService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnCheckAdministratorRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAccountRolesService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountRolesRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadSupervisedAccountRolesService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadSupervisedAccountRolesRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    LoadParentTradingGroupService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadParentTradingGroupRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAccountIdentityService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountIdentityRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    StoreAccountIdentityService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnStoreAccountIdentityRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    LoadTradingGroupService::AddSlot(Store(slots),
      std::bind(&AdministrationServlet::OnLoadTradingGroupRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAdministratorsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAdministratorsRequest, this,
      std::placeholders::_1));
    LoadServicesService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadServicesRequest, this,
      std::placeholders::_1));
    LoadEntitlementsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadEntitlementsRequest, this,
      std::placeholders::_1));
    LoadAccountEntitlementsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountEntitlementsRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    StoreEntitlementsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnStoreEntitlementsRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    MonitorRiskParametersService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnMonitorRiskParametersRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    StoreRiskParametersService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnStoreRiskParametersRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    MonitorRiskStateService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnMonitorRiskStateRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    StoreRiskStateService::AddRequestSlot(Store(slots), std::bind(
      &AdministrationServlet::OnStoreRiskStateRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    LoadManagedTradingGroupsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadManagedTradingGroupsRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAccountModificationRequestService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountModificationRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAccountModificationRequestIdsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountModificationRequestIds, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4));
    LoadManagedAccountModificationRequestIdsService::AddSlot(Store(slots),
      std::bind(
      &AdministrationServlet::OnLoadManagedAccountModificationRequestIds, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4));
    LoadEntitlementModificationService::AddSlot(Store(slots),
      std::bind(&AdministrationServlet::OnLoadEntitlementModification, this,
      std::placeholders::_1, std::placeholders::_2));
    SubmitEntitlementModificationRequestService::AddSlot(Store(slots),
      std::bind(&AdministrationServlet::OnSubmitEntitlementModificationRequest,
      this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4));
    LoadRiskModificationService::AddSlot(Store(slots),
      std::bind(&AdministrationServlet::OnLoadRiskModification, this,
      std::placeholders::_1, std::placeholders::_2));
    SubmitRiskModificationRequestService::AddSlot(Store(slots),
      std::bind(&AdministrationServlet::OnSubmitRiskModificationRequest,
      this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4));
    LoadAccountModificationRequestStatusService::AddSlot(Store(slots),
      std::bind(&AdministrationServlet::OnLoadAccountModificationRequestStatus,
      this, std::placeholders::_1, std::placeholders::_2));
    ApproveAccountModificationRequestService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnApproveAccountModificationRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    RejectAccountModificationRequestService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnRejectAccountModificationRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    LoadMessageService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadMessage, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadMessageIdsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadMessageIds, this, std::placeholders::_1,
      std::placeholders::_2));
    SendAccountModificationRequestMessageService::AddSlot(Store(slots),
      std::bind(&AdministrationServlet::OnSendAccountModificationRequestMessage,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::HandleClientClosed(
      ServiceProtocolClient& client) {
    Beam::Threading::With(m_riskParametersSubscribers,
      [&] (auto& accountToSubscribers) {
        for(auto& subscribers : accountToSubscribers |
            boost::adaptors::map_values) {
          Beam::Threading::With(subscribers, [&] (auto& subscribers) {
            Beam::RemoveAll(subscribers, &client);
          });
        }
      });
    Beam::Threading::With(m_riskStateEntries, [&] (auto& riskStateEntries) {
      for(auto& entry : riskStateEntries | boost::adaptors::map_values) {
        Beam::RemoveAll(entry.m_subscribers, &client);
      }
    });
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_openState.Close();
  }

  template<typename C, typename S, typename D>
  AccountRoles AdministrationServlet<C, S, D>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto roles = AccountRoles();
    auto parents = m_serviceLocatorClient->LoadParents(account);
    auto tradingGroups = m_serviceLocatorClient->LoadChildren(
      m_tradingGroupsRoot);
    for(auto& parent : parents) {
      if(parent == m_administratorsRoot) {
        roles.Set(AccountRole::ADMINISTRATOR);
      } else if(parent == m_servicesRoot) {
        roles.Set(AccountRole::SERVICE);
      } else if(!roles.Test(AccountRole::TRADER) &&
          parent.m_name == "traders") {
        auto entryParents = m_serviceLocatorClient->LoadParents(parent);
        for(auto& entryParent : entryParents) {
          if(std::find(tradingGroups.begin(), tradingGroups.end(),
              entryParent) != tradingGroups.end()) {
            roles.Set(AccountRole::TRADER);
            break;
          }
        }
      } else if(!roles.Test(AccountRole::MANAGER) &&
          parent.m_name == "managers") {
        auto entryParents = m_serviceLocatorClient->LoadParents(parent);
        for(auto& entryParent : entryParents) {
          if(std::find(tradingGroups.begin(), tradingGroups.end(),
              entryParent) != tradingGroups.end()) {
            roles.Set(AccountRole::MANAGER);
            break;
          }
        }
      }
    }
    return roles;
  }

  template<typename C, typename S, typename D>
  AccountRoles AdministrationServlet<C, S, D>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    if(parent == child) {
      return LoadAccountRoles(child);
    }
    auto roles = AccountRoles();
    if(CheckAdministrator(parent)) {
      roles.Set(AccountRole::ADMINISTRATOR);
    }
    auto tradingGroupEntries = LoadManagedTradingGroups(parent);
    auto parents = m_serviceLocatorClient->LoadParents(child);
    for(auto& tradingGroupEntry : tradingGroupEntries) {
      auto managersGroup = m_serviceLocatorClient->LoadDirectoryEntry(
        tradingGroupEntry, "managers");
      if(std::find(parents.begin(), parents.end(), managersGroup) !=
          parents.end()) {
        roles.Set(AccountRole::MANAGER);
        break;
      }
      auto tradersGroup = m_serviceLocatorClient->LoadDirectoryEntry(
        tradingGroupEntry, "traders");
      if(std::find(parents.begin(), parents.end(), tradersGroup) !=
          parents.end()) {
        roles.Set(AccountRole::MANAGER);
        break;
      }
    }
    return roles;
  }

  template<typename C, typename S, typename D>
  bool AdministrationServlet<C, S, D>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_serviceLocatorClient->LoadParents(account);
    auto isAdministrator = std::find(parents.begin(), parents.end(),
      m_administratorsRoot) != parents.end();
    return isAdministrator;
  }

  template<typename C, typename S, typename D>
  bool AdministrationServlet<C, S, D>::CheckReadPermission(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    if(parent == child) {
      return true;
    }
    if(CheckAdministrator(parent)) {
      return true;
    }
    auto tradingGroups = LoadManagedTradingGroups(parent);
    auto parentGroups = m_serviceLocatorClient->LoadParents(child);
    for(auto& tradingGroup : tradingGroups) {
      auto traders = m_serviceLocatorClient->LoadDirectoryEntry(tradingGroup,
        "traders");
      if(std::find(parentGroups.begin(), parentGroups.end(), traders) !=
          parentGroups.end()) {
        return true;
      }
      auto managers = m_serviceLocatorClient->LoadDirectoryEntry(tradingGroup,
        "managers");
      if(std::find(parentGroups.begin(), parentGroups.end(), managers) !=
          parentGroups.end()) {
        return true;
      }
    }
    return false;
  }

  template<typename C, typename S, typename D>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_serviceLocatorClient->LoadParents(account);
    auto tradingGroups = m_serviceLocatorClient->LoadChildren(
      m_tradingGroupsRoot);
    for(auto& parent : parents) {
      if(parent == m_administratorsRoot) {
        return tradingGroups;
      }
    }
    auto result = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    for(auto& parent : parents) {
      if(parent.m_name == "managers") {
        auto entryParents = m_serviceLocatorClient->LoadParents(parent);
        for(auto& entryParent : entryParents) {
          if(std::find(tradingGroups.begin(), tradingGroups.end(),
              entryParent) != tradingGroups.end()) {
            result.push_back(entryParent);
            break;
          }
        }
      }
    }
    return result;
  }

  template<typename C, typename S, typename D>
  TradingGroup AdministrationServlet<C, S, D>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto managersDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      directory, "managers");
    auto tradersDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      directory, "traders");
    auto managers = m_serviceLocatorClient->LoadChildren(managersDirectory);
    auto traders = m_serviceLocatorClient->LoadChildren(tradersDirectory);
    auto tradingGroup = TradingGroup(directory, managersDirectory, managers,
      tradersDirectory, traders);
    return tradingGroup;
  }

  template<typename C, typename S, typename D>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D>::LoadEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_serviceLocatorClient->LoadParents(account);
    auto result = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    auto& availableEntitlements = m_entitlements.GetEntries();
    for(auto& availableEntitlement : availableEntitlements) {
      auto entryIterator = std::find(parents.begin(), parents.end(),
        availableEntitlement.m_groupEntry);
      if(entryIterator != parents.end()) {
        result.push_back(*entryIterator);
      }
    }
    return result;
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::GrantEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& adminAccount,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto existingEntitlements = LoadEntitlements(account);
    auto entitlementSet =
      std::unordered_set<Beam::ServiceLocator::DirectoryEntry>(
      entitlements.begin(), entitlements.end());
    for(auto& entitlement : m_entitlements.GetEntries()) {
      auto& entry = entitlement.m_groupEntry;
      if(entitlementSet.find(entry) != entitlementSet.end()) {
        if(std::find(existingEntitlements.begin(),
            existingEntitlements.end(), entry) == existingEntitlements.end()) {
          m_serviceLocatorClient->Associate(account, entry);
          auto ss = std::stringstream();
          ss << boost::posix_time::to_simple_string(
            boost::posix_time::second_clock::universal_time()) << ": " <<
            adminAccount.m_name << " grants entitlement \"" <<
            entitlement.m_name << "\"" << " to " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      } else {
        if(std::find(existingEntitlements.begin(),
            existingEntitlements.end(), entry) != existingEntitlements.end()) {
          m_serviceLocatorClient->Detach(account, entry);
          auto ss = std::stringstream();
          ss << boost::posix_time::to_simple_string(
            boost::posix_time::second_clock::universal_time()) << ": " <<
            adminAccount.m_name << " revokes entitlement \"" <<
            entitlement.m_name << "\"" << " from " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      }
    }
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::UpdateRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& parameters) {
    auto& subscribers = Beam::Threading::With(m_riskParametersSubscribers,
      [&] (auto& accountToSubscribers) -> decltype(auto) {
        return accountToSubscribers[account];
      });
    m_dataStore->WithTransaction([&] {
      m_dataStore->Store(account, parameters);
      Beam::Threading::With(subscribers, [&] (auto& subscribers) {
        for(auto& subscriber : subscribers) {
          Beam::Services::SendRecordMessage<RiskParametersMessage>(*subscriber,
            account, parameters);
        }
      });
    });
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::EnsureModificationReadPermission(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id id) {
    auto request = AccountModificationRequest();
    m_dataStore->WithTransaction([&] {
      request = m_dataStore->LoadAccountModificationRequest(id);
    });
    if(!CheckReadPermission(account, request.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
  }

  template<typename C, typename S, typename D>
  AccountModificationRequest AdministrationServlet<C, S, D>::
      MakeModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& sessionAccount,
      const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountRoles& roles, AccountModificationRequest::Type type) {
    if(!CheckReadPermission(sessionAccount, submissionAccount) ||
        roles.GetBitset().none()) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto requestId = ++m_nextModificationRequestId;
    auto timestamp = boost::posix_time::second_clock::universal_time();
    return {requestId, type, account, submissionAccount, timestamp};
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::StoreModificationRequest(
      const AccountModificationRequest& request, const Message& comment,
      const AccountRoles& roles) {
    if(comment.GetBodies().size() > 1 || !comment.GetBody().m_message.empty()) {
      auto message = Message{++m_nextMessageId, request.GetSubmissionAccount(),
        request.GetTimestamp(), comment.GetBodies()};
      m_dataStore->Store(request.GetId(), message);
    }
    if(roles.Test(AccountRole::ADMINISTRATOR)) {
      auto update = AccountModificationRequest::Update{
        AccountModificationRequest::Status::GRANTED,
        request.GetSubmissionAccount(), 0, request.GetTimestamp()};
      m_dataStore->Store(request.GetId(), update);
    } else if(roles.Test(AccountRole::MANAGER)) {
      auto update = AccountModificationRequest::Update{
        AccountModificationRequest::Status::REVIEWED,
        request.GetSubmissionAccount(), 0, request.GetTimestamp()};
      m_dataStore->Store(request.GetId(), update);
    } else {
      auto update = AccountModificationRequest::Update{
        AccountModificationRequest::Status::PENDING,
        request.GetSubmissionAccount(), 0, request.GetTimestamp()};
      m_dataStore->Store(request.GetId(), update);
    }
  }

  template<typename C, typename S, typename D>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D>::OnLoadAccountsByRoles(
      ServiceProtocolClient& client, AccountRoles roles) {
    auto& session = client.GetSession();
    auto accounts = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    if(CheckAdministrator(session.GetAccount())) {
      if(roles.Test(AccountRole::ADMINISTRATOR)) {
        auto administrators = m_serviceLocatorClient->LoadChildren(
          m_administratorsRoot);
        for(auto& administrator : administrators) {
          if(administrator.m_type ==
              Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT) {
            accounts.push_back(std::move(administrator));
          }
        }
      }
      if(roles.Test(AccountRole::SERVICE)) {
        auto serviceAccounts = m_serviceLocatorClient->LoadChildren(
          m_servicesRoot);
        for(auto& serviceAccount : serviceAccounts) {
          if(serviceAccount.m_type ==
              Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT) {
            if(std::find(accounts.begin(), accounts.end(), serviceAccount) ==
                accounts.end()) {
              accounts.push_back(std::move(serviceAccount));
            }
          }
        }
      }
    }
    return accounts;
  }

  template<typename C, typename S, typename D>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D>::
      OnLoadAdministratorsRootEntry(ServiceProtocolClient& client) {
    return m_administratorsRoot;
  }

  template<typename C, typename S, typename D>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D>::
      OnLoadServicesRootEntry(ServiceProtocolClient& client) {
    return m_servicesRoot;
  }

  template<typename C, typename S, typename D>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D>::
      OnLoadTradingGroupsRootEntry(ServiceProtocolClient& client) {
    return m_tradingGroupsRoot;
  }

  template<typename C, typename S, typename D>
  bool AdministrationServlet<C, S, D>::OnCheckAdministratorRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return CheckAdministrator(account);
  }

  template<typename C, typename S, typename D>
  AccountRoles AdministrationServlet<C, S, D>::OnLoadAccountRolesRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return LoadAccountRoles(account);
  }

  template<typename C, typename S, typename D>
  AccountRoles AdministrationServlet<C, S, D>::
      OnLoadSupervisedAccountRolesRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return LoadAccountRoles(parent, child);
  }

  template<typename C, typename S, typename D>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D>::
      OnLoadParentTradingGroupRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_serviceLocatorClient->LoadParents(account);
    auto tradingGroups = m_serviceLocatorClient->LoadChildren(
      m_tradingGroupsRoot);
    for(auto& parent : parents) {
      if(parent.m_name == "traders") {
        auto entryParents = m_serviceLocatorClient->LoadParents(parent);
        for(auto& entryParent : entryParents) {
          if(std::find(tradingGroups.begin(), tradingGroups.end(),
              entryParent) != tradingGroups.end()) {
            return entryParent;
          }
        }
      }
    }
    return {};
  }

  template<typename C, typename S, typename D>
  AccountIdentity AdministrationServlet<C, S, D>::OnLoadAccountIdentityRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto identity = AccountIdentity();
    m_dataStore->WithTransaction([&] {
      identity = m_dataStore->LoadIdentity(account);
    });
    identity.m_lastLoginTime = m_serviceLocatorClient->LoadLastLoginTime(
      account);
    identity.m_registrationTime = m_serviceLocatorClient->LoadRegistrationTime(
      account);
    return identity;
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::OnStoreAccountIdentityRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto accountEntry = m_serviceLocatorClient->LoadDirectoryEntry(
      account.m_id);
    m_dataStore->WithTransaction([&] {
      m_dataStore->Store(accountEntry, identity);
    });
  }

  template<typename C, typename S, typename D>
  TradingGroup AdministrationServlet<C, S, D>::OnLoadTradingGroupRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto properDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      directory.m_id);
    return LoadTradingGroup(properDirectory);
  }

  template<typename C, typename S, typename D>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D>::OnLoadAdministratorsRequest(
      ServiceProtocolClient& client) {
    return m_serviceLocatorClient->LoadChildren(m_administratorsRoot);
  }

  template<typename C, typename S, typename D>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D>::OnLoadServicesRequest(
      ServiceProtocolClient& client) {
    auto servicesDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(), "services");
    return m_serviceLocatorClient->LoadChildren(servicesDirectory);
  }

  template<typename C, typename S, typename D>
  MarketDataService::EntitlementDatabase AdministrationServlet<C, S, D>::
      OnLoadEntitlementsRequest(ServiceProtocolClient& client) {
    return m_entitlements;
  }

  template<typename C, typename S, typename D>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D>::OnLoadAccountEntitlementsRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return LoadEntitlements(account);
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::OnStoreEntitlementsRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto& session = client.GetSession();
    if(!CheckAdministrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    GrantEntitlements(session.GetAccount(), account, entitlements);
  }

  template<typename C, typename S, typename D>
  RiskService::RiskParameters AdministrationServlet<C, S, D>::
      OnMonitorRiskParametersRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& syncSubscribers = Beam::Threading::With(m_riskParametersSubscribers,
      [&] (auto& accountToSubscribers) -> decltype(auto) {
        return accountToSubscribers[account];
      });
    Beam::Threading::With(syncSubscribers, [&] (auto& subscribers) {
      if(std::find(subscribers.begin(), subscribers.end(), &client) ==
          subscribers.end()) {
        subscribers.push_back(&client);
      }
    });
    auto parameters = RiskService::RiskParameters();
    m_dataStore->WithTransaction([&] {
      parameters = m_dataStore->LoadRiskParameters(account);
    });
    return parameters;
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::OnStoreRiskParametersRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& parameters) {
    auto& session = client.GetSession();
    if(!CheckAdministrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    UpdateRiskParameters(account, parameters);
  }

  template<typename C, typename S, typename D>
  RiskService::RiskState AdministrationServlet<C, S, D>::
      OnMonitorRiskStateRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto riskState = RiskService::RiskState();
    m_dataStore->WithTransaction([&] {
      Beam::Threading::With(m_riskStateEntries, [&] (auto& riskStateEntries) {
        auto& riskStateEntry = riskStateEntries[account];
        riskState = m_dataStore->LoadRiskState(account);
        if(std::find(riskStateEntry.m_subscribers.begin(),
            riskStateEntry.m_subscribers.end(), &client) !=
            riskStateEntry.m_subscribers.end()) {
          return;
        }
        riskStateEntry.m_subscribers.push_back(&client);
      });
    });
    return riskState;
  }

  template<typename C, typename S, typename D>
  void AdministrationServlet<C, S, D>::OnStoreRiskStateRequest(
      Beam::Services::RequestToken<ServiceProtocolClient,
      StoreRiskStateService>& request,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto& session = request.GetSession();
    if(!CheckAdministrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    m_dataStore->WithTransaction([&] {
      m_dataStore->Store(account, riskState);
      request.SetResult();
      Beam::Threading::With(m_riskStateEntries, [&] (auto& riskStateEntries) {
        auto riskStateEntryIterator = riskStateEntries.find(account);
        if(riskStateEntryIterator == riskStateEntries.end()) {
          return;
        }
        auto& riskStateEntry = riskStateEntryIterator->second;
        riskStateEntry.m_riskState = riskState;
        for(auto& subscriber : riskStateEntry.m_subscribers) {
          Beam::Services::SendRecordMessage<RiskStateMessage>(*subscriber,
            account, riskState);
        }
      });
    });
  }

  template<typename C, typename S, typename D>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D>::OnLoadManagedTradingGroupsRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return LoadManagedTradingGroups(account);
  }

  template<typename C, typename S, typename D>
  AccountModificationRequest AdministrationServlet<C, S, D>::
      OnLoadAccountModificationRequest(ServiceProtocolClient& client,
      AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    auto request = AccountModificationRequest();
    m_dataStore->WithTransaction([&] {
      request = m_dataStore->LoadAccountModificationRequest(id);
    });
    if(!CheckReadPermission(session.GetAccount(), request.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return request;
  }

  template<typename C, typename S, typename D>
  std::vector<AccountModificationRequest::Id>
      AdministrationServlet<C, S, D>::OnLoadAccountModificationRequestIds(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    auto& session = client.GetSession();
    if(!CheckReadPermission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto ids = std::vector<AccountModificationRequest::Id>();
    m_dataStore->WithTransaction([&] {
      ids = m_dataStore->LoadAccountModificationRequestIds(account, startId,
        maxCount);
    });
    return ids;
  }

  template<typename C, typename S, typename D>
  std::vector<AccountModificationRequest::Id> AdministrationServlet<C, S, D>::
      OnLoadManagedAccountModificationRequestIds(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    auto& session = client.GetSession();
    if(!CheckReadPermission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    if(CheckAdministrator(session.GetAccount())) {
      auto ids = std::vector<AccountModificationRequest::Id>();
      m_dataStore->WithTransaction([&] {
        ids = m_dataStore->LoadAccountModificationRequestIds(startId, maxCount);
      });
      return ids;
    }
    auto tradingGroupEntries = LoadManagedTradingGroups(account);
    auto accounts = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    for(auto& tradingGroupEntry : tradingGroupEntries) {
      auto tradingGroup = LoadTradingGroup(tradingGroupEntry);
      std::move(tradingGroup.GetManagers().begin(),
        tradingGroup.GetManagers().end(), std::back_inserter(accounts));
      std::move(tradingGroup.GetTraders().begin(),
        tradingGroup.GetTraders().end(), std::back_inserter(accounts));
    }
    std::sort(accounts.begin(), accounts.end());
    auto ids = std::vector<AccountModificationRequest::Id>();
    for(auto i = std::size_t(0); i != accounts.size(); ++i) {
      if(i != 0 && accounts[i] == accounts[i - 1]) {
        continue;
      }
      auto accountRequestIds = std::vector<AccountModificationRequest::Id>();
      m_dataStore->WithTransaction([&] {
        accountRequestIds = m_dataStore->LoadAccountModificationRequestIds(
          accounts[i], startId, maxCount);
      });
      ids.insert(ids.end(), accountRequestIds.begin(), accountRequestIds.end());
    }
    std::sort(ids.begin(), ids.end());
    if(static_cast<int>(ids.size()) > maxCount) {
      ids.erase(ids.begin() + maxCount, ids.end());
    }
    return ids;
  }

  template<typename C, typename S, typename D>
  EntitlementModification AdministrationServlet<C, S, D>::
      OnLoadEntitlementModification(ServiceProtocolClient& client,
      AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    EnsureModificationReadPermission(session.GetAccount(), id);
    auto modification = EntitlementModification();
    m_dataStore->WithTransaction([&] {
      modification = m_dataStore->LoadEntitlementModification(id);
    });
    return modification;
  }

  template<typename C, typename S, typename D>
  AccountModificationRequest AdministrationServlet<C, S, D>::
      OnSubmitEntitlementModificationRequest(ServiceProtocolClient& client,
      Beam::ServiceLocator::DirectoryEntry account,
      const EntitlementModification& modification, Message comment) {
    auto& session = client.GetSession();
    if(account.m_id == -1) {
      account = session.GetAccount();
    }
    auto roles = LoadAccountRoles(session.GetAccount(), account);
    auto request = MakeModificationRequest(session.GetAccount(),
      session.GetAccount(), account, roles,
      AccountModificationRequest::Type::ENTITLEMENTS);
    for(auto& entitlement : modification.GetEntitlements()) {
      if(entitlement.m_type !=
          Beam::ServiceLocator::DirectoryEntry::Type::DIRECTORY) {
        throw Beam::Services::ServiceRequestException("Invalid entitlement.");
      }
      if(std::find_if(m_entitlements.GetEntries().begin(),
          m_entitlements.GetEntries().end(), [&] (auto& entry) {
            return entry.m_groupEntry == entitlement;
          }) == m_entitlements.GetEntries().end()) {
        throw Beam::Services::ServiceRequestException("Invalid entitlement.");
      }
    }
    m_dataStore->WithTransaction([&] {
      m_dataStore->Store(request, modification);
      StoreModificationRequest(request, comment, roles);
    });
    if(roles.Test(AccountRole::ADMINISTRATOR)) {
      GrantEntitlements(request.GetSubmissionAccount(),
        request.GetAccount(), modification.GetEntitlements());
    }
    return request;
  }

  template<typename C, typename S, typename D>
  RiskModification AdministrationServlet<C, S, D>::OnLoadRiskModification(
      ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    EnsureModificationReadPermission(session.GetAccount(), id);
    auto modification = RiskModification();
    m_dataStore->WithTransaction([&] {
      modification = m_dataStore->LoadRiskModification(id);
    });
    return modification;
  }

  template<typename C, typename S, typename D>
  AccountModificationRequest AdministrationServlet<C, S, D>::
      OnSubmitRiskModificationRequest(ServiceProtocolClient& client,
      Beam::ServiceLocator::DirectoryEntry account,
      const RiskModification& modification, Message comment) {
    auto& session = client.GetSession();
    if(account.m_id == -1) {
      account = session.GetAccount();
    }
    auto roles = LoadAccountRoles(session.GetAccount(), account);
    auto request = MakeModificationRequest(session.GetAccount(),
      session.GetAccount(), account, roles,
      AccountModificationRequest::Type::RISK);
    m_dataStore->WithTransaction([&] {
      m_dataStore->Store(request, modification);
      StoreModificationRequest(request, comment, roles);
    });
    if(roles.Test(AccountRole::ADMINISTRATOR)) {
      UpdateRiskParameters(request.GetAccount(), modification.GetParameters());
    }
    return request;
  }

  template<typename C, typename S, typename D>
  AccountModificationRequest::Update AdministrationServlet<C, S, D>::
      OnLoadAccountModificationRequestStatus(ServiceProtocolClient& client,
      AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    auto request = AccountModificationRequest();
    m_dataStore->WithTransaction([&] {
      request = m_dataStore->LoadAccountModificationRequest(id);
    });
    if(!CheckReadPermission(session.GetAccount(), request.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto status = AccountModificationRequest::Update();
    m_dataStore->WithTransaction([&] {
      status = m_dataStore->LoadAccountModificationRequestStatus(id);
    });
    return status;
  }

  template<typename C, typename S, typename D>
  AccountModificationRequest::Update AdministrationServlet<C, S, D>::
      OnApproveAccountModificationRequest(ServiceProtocolClient& client,
      AccountModificationRequest::Id id, Message comment) {
    auto& session = client.GetSession();
    auto request = AccountModificationRequest();
    m_dataStore->WithTransaction([&] {
      request = m_dataStore->LoadAccountModificationRequest(id);
    });
    auto roles = LoadAccountRoles(session.GetAccount(), request.GetAccount());
    if(roles.GetBitset().none()) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto timestamp = boost::posix_time::second_clock::universal_time();
    if(comment.GetBodies().size() == 1 && comment.GetBody().m_message.empty()) {
      comment = Message{-1, {}, {}, {}};
    } else {
      comment = Message{++m_nextMessageId, session.GetAccount(), timestamp,
        comment.GetBodies()};
    }
    auto update = AccountModificationRequest::Update();
    m_dataStore->WithTransaction([&] {
      update = m_dataStore->LoadAccountModificationRequestStatus(
        request.GetId());
      if(IsTerminal(update.m_status)) {
        throw Beam::Services::ServiceRequestException(
          "Request can not be updated.");
      }
      if(comment.GetId() != -1) {
        m_dataStore->Store(request.GetId(), comment);
      }
      if(roles.Test(AccountRole::ADMINISTRATOR)) {
        update.m_status = AccountModificationRequest::Status::GRANTED;
      } else if(roles.Test(AccountRole::MANAGER)) {
        update.m_status = AccountModificationRequest::Status::REVIEWED;
      }
      update.m_account = session.GetAccount();
      ++update.m_sequenceNumber;
      update.m_timestamp = timestamp;
      m_dataStore->Store(request.GetId(), update);
    });
    if(update.m_status == AccountModificationRequest::Status::GRANTED) {
      if(request.GetType() == AccountModificationRequest::Type::ENTITLEMENTS) {
        auto modification = EntitlementModification();
        m_dataStore->WithTransaction([&] {
          modification = m_dataStore->LoadEntitlementModification(
            request.GetId());
        });
        GrantEntitlements(update.m_account, session.GetAccount(),
          modification.GetEntitlements());
      } else if(request.GetType() == AccountModificationRequest::Type::RISK) {
        auto modification = RiskModification();
        m_dataStore->WithTransaction([&] {
          modification = m_dataStore->LoadRiskModification(request.GetId());
        });
        UpdateRiskParameters(request.GetAccount(),
          modification.GetParameters());
      }
    }
    return update;
  }

  template<typename C, typename S, typename D>
  AccountModificationRequest::Update AdministrationServlet<C, S, D>::
      OnRejectAccountModificationRequest(ServiceProtocolClient& client,
      AccountModificationRequest::Id id, Message comment) {
    auto& session = client.GetSession();
    auto request = AccountModificationRequest();
    m_dataStore->WithTransaction([&] {
      request = m_dataStore->LoadAccountModificationRequest(id);
    });
    auto roles = LoadAccountRoles(session.GetAccount(), request.GetAccount());
    if(roles.GetBitset().none()) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto timestamp = boost::posix_time::second_clock::universal_time();
    if(comment.GetBodies().size() == 1 && comment.GetBody().m_message.empty()) {
      comment = Message{-1, {}, {}, {}};
    } else {
      comment = Message{++m_nextMessageId, session.GetAccount(), timestamp,
        comment.GetBodies()};
    }
    auto update = AccountModificationRequest::Update();
    m_dataStore->WithTransaction([&] {
      update = m_dataStore->LoadAccountModificationRequestStatus(
        request.GetId());
      if(IsTerminal(update.m_status)) {
        throw Beam::Services::ServiceRequestException(
          "Request can not be updated.");
      }
      if(comment.GetId() != -1) {
        m_dataStore->Store(request.GetId(), comment);
      }
      update.m_status = AccountModificationRequest::Status::REJECTED;
      update.m_account = session.GetAccount();
      ++update.m_sequenceNumber;
      update.m_timestamp = timestamp;
      m_dataStore->Store(request.GetId(), update);
    });
    return update;
  }

  template<typename C, typename S, typename D>
  Message AdministrationServlet<C, S, D>::OnLoadMessage(
      ServiceProtocolClient& client, Message::Id id) {
    auto& session = client.GetSession();
    auto message = Message();
    m_dataStore->WithTransaction([&] {
      message = m_dataStore->LoadMessage(id);
    });
    return message;
  }

  template<typename C, typename S, typename D>
  std::vector<Message::Id> AdministrationServlet<C, S, D>::OnLoadMessageIds(
      ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    auto request = AccountModificationRequest();
    m_dataStore->WithTransaction([&] {
      request = m_dataStore->LoadAccountModificationRequest(id);
    });
    if(!CheckReadPermission(session.GetAccount(), request.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto messageIds = std::vector<Message::Id>();
    m_dataStore->WithTransaction([&] {
      messageIds = m_dataStore->LoadMessageIds(id);
    });
    return messageIds;
  }

  template<typename C, typename S, typename D>
  Message AdministrationServlet<C, S, D>::
      OnSendAccountModificationRequestMessage(ServiceProtocolClient& client,
      AccountModificationRequest::Id id, Message message) {
    auto& session = client.GetSession();
    auto account = message.GetAccount();
    if(account.m_id == -1) {
      account = session.GetAccount();
    } else {
      if(!CheckReadPermission(session.GetAccount(), message.GetAccount())) {
        throw Beam::Services::ServiceRequestException(
          "Insufficient permissions.");
      }
    }
    auto timestamp = boost::posix_time::second_clock::universal_time();
    message = Message{++m_nextMessageId, account, timestamp,
      message.GetBodies()};
    m_dataStore->WithTransaction([&] {
      m_dataStore->Store(id, message);
    });
    return message;
  }
}

#endif
