#ifndef NEXUS_ADMINISTRATIONSERVLET_HPP
#define NEXUS_ADMINISTRATIONSERVLET_HPP
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/AdministrationServices.hpp"
#include "Nexus/AdministrationService/AdministrationSession.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class AdministrationServlet
      \brief Provides management and administration services for Nexus accounts.
      \tparam ContainerType The container instantiating this servlet.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient used to
              manage accounts.
      \tparam AdministrationDataStoreType The type of AdministrationDataStore to
              use.
   */
  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  class AdministrationServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of ServiceLocatorClient used to manage accounts.
      using ServiceLocatorClient =
        Beam::GetTryDereferenceType<ServiceLocatorClientType>;

      //! The type of AdministrationDataStore used.
      using AdministrationDataStore =
        Beam::GetTryDereferenceType<AdministrationDataStoreType>;

      //! Constructs an AdministrationServlet.
      /*!
        \param serviceLocatorClient Initializes the ServiceLocatorClient.
        \param entitlements The list of available entitlements.
        \param dataStore Initializes the AdministrationDataStore.
      */
      template<typename ServiceLocatorClientForward,
        typename AdministrationDataStoreForward>
      AdministrationServlet(ServiceLocatorClientForward&& serviceLocatorClient,
        const MarketDataService::EntitlementDatabase& entitlements,
        AdministrationDataStoreForward&& dataStore);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

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
      Beam::GetOptionalLocalPtr<ServiceLocatorClientType>
        m_serviceLocatorClient;
      MarketDataService::EntitlementDatabase m_entitlements;
      Beam::GetOptionalLocalPtr<AdministrationDataStoreType> m_dataStore;
      Beam::ServiceLocator::DirectoryEntry m_administratorsRoot;
      Beam::ServiceLocator::DirectoryEntry m_servicesRoot;
      Beam::ServiceLocator::DirectoryEntry m_tradingGroupsRoot;
      SyncAccountToSubscribers m_riskParametersSubscribers;
      SyncRiskStateEntries m_riskStateEntries;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);
      bool OnCheckAdministratorRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles OnLoadAccountRolesRequest(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      Beam::ServiceLocator::DirectoryEntry
        OnLoadAccountTradingGroupEntryRequest(ServiceProtocolClient& client,
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
        OnLoadAdministratorsRequest(ServiceProtocolClient& client, int dummy);
      std::vector<Beam::ServiceLocator::DirectoryEntry> OnLoadServicesRequest(
        ServiceProtocolClient& client, int dummy);
      MarketDataService::EntitlementDatabase OnLoadEntitlementsRequest(
        ServiceProtocolClient& client, int dummy);
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
      void OnStoreRiskParametersRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, StoreRiskParametersService>& request,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);
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
  };

  template<typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  struct MetaAdministrationServlet {
    using Session = AdministrationSession;
    template<typename ContainerType>
    struct apply {
      using type = AdministrationServlet<ContainerType,
        ServiceLocatorClientType, AdministrationDataStoreType>;
    };
  };

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  template<typename ServiceLocatorClientForward,
    typename AdministrationDataStoreForward>
  AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::AdministrationServlet(
      ServiceLocatorClientForward&& serviceLocatorClient,
      const MarketDataService::EntitlementDatabase& entitlements,
      AdministrationDataStoreForward&& dataStore)
      : m_serviceLocatorClient(std::forward<ServiceLocatorClientForward>(
          serviceLocatorClient)),
        m_entitlements(entitlements),
        m_dataStore(std::forward<AdministrationDataStoreForward>(
          dataStore)) {}

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>>
      slots) {
    RegisterAdministrationServices(Store(slots));
    RegisterAdministrationMessages(Store(slots));
    CheckAdministratorService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnCheckAdministratorRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAccountRolesService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountRolesRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAccountTradingGroupEntryService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountTradingGroupEntryRequest, this,
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
      std::placeholders::_1, std::placeholders::_2));
    LoadServicesService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadServicesRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadEntitlementsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadEntitlementsRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadAccountEntitlementsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnLoadAccountEntitlementsRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    StoreEntitlementsService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnStoreEntitlementsRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    MonitorRiskParametersService::AddSlot(Store(slots), std::bind(
      &AdministrationServlet::OnMonitorRiskParametersRequest, this,
      std::placeholders::_1, std::placeholders::_2));
    StoreRiskParametersService::AddRequestSlot(Store(slots), std::bind(
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
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    Beam::Threading::With(m_riskParametersSubscribers,
      [&] (AccountToRiskSubscribers& accountToSubscribers) {
        for(auto& subscribers : accountToSubscribers |
            boost::adaptors::map_values) {
          Beam::Threading::With(subscribers,
            [&] (RiskParameterSubscribers& subscribers) {
              Beam::RemoveAll(subscribers, &client);
            });
        }
      });
    Beam::Threading::With(m_riskStateEntries,
      [&] (RiskStateEntries& riskStateEntries) {
        for(auto& entry : riskStateEntries | boost::adaptors::map_values) {
          Beam::RemoveAll(entry.m_subscribers, &client);
        }
      });
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorClient->Open();
      m_dataStore->Open();
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
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::Shutdown() {
    m_dataStore->Close();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  bool AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_serviceLocatorClient->LoadParents(account);
    auto isAdministrator = std::find(parents.begin(), parents.end(),
      m_administratorsRoot) != parents.end();
    return isAdministrator;
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationServlet<
      ContainerType, ServiceLocatorClientType, AdministrationDataStoreType>::
      LoadEntitlements(const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_serviceLocatorClient->LoadParents(account);
    std::vector<Beam::ServiceLocator::DirectoryEntry> result;
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

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  bool AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnCheckAdministratorRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return CheckAdministrator(account);
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  AccountRoles AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnLoadAccountRolesRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    AccountRoles roles;
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

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<ContainerType,
      ServiceLocatorClientType, AdministrationDataStoreType>::
      OnLoadAccountTradingGroupEntryRequest(ServiceProtocolClient& client,
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
    return Beam::ServiceLocator::DirectoryEntry();
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  AccountIdentity AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnLoadAccountIdentityRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    AccountIdentity identity;
    m_dataStore->WithTransaction(
      [&] {
        identity = m_dataStore->LoadIdentity(account);
      });
    identity.m_lastLoginTime = m_serviceLocatorClient->LoadLastLoginTime(
      account);
    identity.m_registrationTime = m_serviceLocatorClient->LoadRegistrationTime(
      account);
    return identity;
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnStoreAccountIdentityRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto accountEntry = m_serviceLocatorClient->LoadDirectoryEntry(
      account.m_id);
    m_dataStore->WithTransaction(
      [&] {
        m_dataStore->Store(accountEntry, identity);
      });
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  TradingGroup AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnLoadTradingGroupRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto properDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      directory.m_id);
    auto managersDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      properDirectory, "managers");
    auto tradersDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      properDirectory, "traders");
    auto managers = m_serviceLocatorClient->LoadChildren(managersDirectory);
    auto traders = m_serviceLocatorClient->LoadChildren(tradersDirectory);
    TradingGroup tradingGroup(properDirectory, managersDirectory, managers,
      tradersDirectory, traders);
    return tradingGroup;
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationServlet<
      ContainerType, ServiceLocatorClientType, AdministrationDataStoreType>::
      OnLoadAdministratorsRequest(ServiceProtocolClient& client, int dummy) {
    return m_serviceLocatorClient->LoadChildren(m_administratorsRoot);
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationServlet<
      ContainerType, ServiceLocatorClientType, AdministrationDataStoreType>::
      OnLoadServicesRequest(ServiceProtocolClient& client, int dummy) {
    auto servicesDirectory = m_serviceLocatorClient->LoadDirectoryEntry(
      Beam::ServiceLocator::DirectoryEntry::GetStarDirectory(), "services");
    return m_serviceLocatorClient->LoadChildren(servicesDirectory);
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  MarketDataService::EntitlementDatabase
      AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnLoadEntitlementsRequest(
      ServiceProtocolClient& client, int dummy) {
    return m_entitlements;
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationServlet<
      ContainerType, ServiceLocatorClientType, AdministrationDataStoreType>::
      OnLoadAccountEntitlementsRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return LoadEntitlements(account);
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnStoreEntitlementsRequest(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto& session = client.GetSession();
    if(!CheckAdministrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto existingEntitlements = LoadEntitlements(account);
    std::unordered_set<Beam::ServiceLocator::DirectoryEntry> entitlementSet(
      entitlements.begin(), entitlements.end());
    for(auto& entitlement : m_entitlements.GetEntries()) {
      auto& entry = entitlement.m_groupEntry;
      if(entitlementSet.find(entry) != entitlementSet.end()) {
        if(std::find(existingEntitlements.begin(),
            existingEntitlements.end(), entry) ==
            existingEntitlements.end()) {
          m_serviceLocatorClient->Associate(account, entry);
          std::stringstream ss;
          ss << boost::posix_time::to_simple_string(
            boost::posix_time::second_clock::universal_time()) << ": " <<
            session.GetAccount().m_name << " grants entitlement \"" <<
            entitlement.m_name << "\"" << " to " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      } else {
        if(std::find(existingEntitlements.begin(),
            existingEntitlements.end(), entry) !=
            existingEntitlements.end()) {
          m_serviceLocatorClient->Detach(account, entry);
          std::stringstream ss;
          ss << boost::posix_time::to_simple_string(
            boost::posix_time::second_clock::universal_time()) << ": " <<
            session.GetAccount().m_name << " revokes entitlement \"" <<
            entitlement.m_name << "\"" << " from " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      }
    }
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  RiskService::RiskParameters AdministrationServlet<ContainerType,
      ServiceLocatorClientType, AdministrationDataStoreType>::
      OnMonitorRiskParametersRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    SyncRiskParameterSubscribers* syncSubscribers;
    Beam::Threading::With(m_riskParametersSubscribers,
      [&] (AccountToRiskSubscribers& accountToSubscribers) {
        syncSubscribers = &(accountToSubscribers[account]);
      });
    Beam::Threading::With(*syncSubscribers,
      [&] (RiskParameterSubscribers& subscribers) {
        if(std::find(subscribers.begin(), subscribers.end(), &client) ==
            subscribers.end()) {
          subscribers.push_back(&client);
        }
      });
    RiskService::RiskParameters parameters;
    m_dataStore->WithTransaction(
      [&] {
        parameters = m_dataStore->LoadRiskParameters(account);
      });
    return parameters;
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnStoreRiskParametersRequest(
      Beam::Services::RequestToken<ServiceProtocolClient,
      StoreRiskParametersService>& request,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    auto& session = request.GetSession();
    if(!CheckAdministrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    SyncRiskParameterSubscribers* syncSubscribers;
    Beam::Threading::With(m_riskParametersSubscribers,
      [&] (AccountToRiskSubscribers& accountToSubscribers) {
        syncSubscribers = &(accountToSubscribers[account]);
      });
    m_dataStore->WithTransaction(
      [&] {
        m_dataStore->Store(account, riskParameters);
        request.SetResult();
        Beam::Threading::With(*syncSubscribers,
          [&] (RiskParameterSubscribers& subscribers) {
            for(auto& subscriber : subscribers) {
              Beam::Services::SendRecordMessage<RiskParametersMessage>(
                *subscriber, account, riskParameters);
            }
          });
      });
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  RiskService::RiskState AdministrationServlet<ContainerType,
      ServiceLocatorClientType, AdministrationDataStoreType>::
      OnMonitorRiskStateRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    RiskService::RiskState riskState;
    auto transaction =
      [&] (RiskStateEntries& riskStateEntries) {
        auto& riskStateEntry = riskStateEntries[account];
        riskState = m_dataStore->LoadRiskState(account);
        if(std::find(riskStateEntry.m_subscribers.begin(),
            riskStateEntry.m_subscribers.end(), &client) !=
            riskStateEntry.m_subscribers.end()) {
          return;
        }
        riskStateEntry.m_subscribers.push_back(&client);
      };
    m_dataStore->WithTransaction(
      [&] {
        Beam::Threading::With(m_riskStateEntries, transaction);
      });
    return riskState;
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  void AdministrationServlet<ContainerType, ServiceLocatorClientType,
      AdministrationDataStoreType>::OnStoreRiskStateRequest(
      Beam::Services::RequestToken<ServiceProtocolClient,
      StoreRiskStateService>& request,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto& session = request.GetSession();
    if(!CheckAdministrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto transaction =
      [&] (RiskStateEntries& riskStateEntries) {
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
      };
    m_dataStore->WithTransaction(
      [&] {
        m_dataStore->Store(account, riskState);
        request.SetResult();
        Beam::Threading::With(m_riskStateEntries, transaction);
      });
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationDataStoreType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationServlet<
      ContainerType, ServiceLocatorClientType, AdministrationDataStoreType>::
      OnLoadManagedTradingGroupsRequest(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_serviceLocatorClient->LoadParents(account);
    auto tradingGroups = m_serviceLocatorClient->LoadChildren(
      m_tradingGroupsRoot);
    for(auto& parent : parents) {
      if(parent == m_administratorsRoot) {
        return tradingGroups;
      }
    }
    std::vector<Beam::ServiceLocator::DirectoryEntry> result;
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
}
}

#endif
