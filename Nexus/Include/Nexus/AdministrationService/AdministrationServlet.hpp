#ifndef NEXUS_ADMINISTRATION_SERVLET_HPP
#define NEXUS_ADMINISTRATION_SERVLET_HPP
#include <atomic>
#include <iostream>
#include <ranges>
#include <sstream>
#include <unordered_set>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include "Nexus/AdministrationService/AdministrationServices.hpp"
#include "Nexus/AdministrationService/AdministrationSession.hpp"

namespace Nexus::AdministrationService {

  /**
   * Provides management and administration services for Nexus accounts.
   * @param <C> The container instantiating this servlet.
   * @param <S> The type of ServiceLocatorClient used to manage accounts.
   * @param <D> The type of AdministrationDataStore to use.
   * @param <R> The type of TimeClient to use.
   */
  template<typename C, typename S, typename D, typename R>
  class AdministrationServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of ServiceLocatorClient used to manage accounts. */
      using ServiceLocatorClient = Beam::GetTryDereferenceType<S>;

      /** The type of AdministrationDataStore used. */
      using AdministrationDataStore = Beam::GetTryDereferenceType<D>;

      /** The type of TimeClient used. */
      using TimeClient = Beam::GetTryDereferenceType<R>;

      /**
       * Constructs an AdministrationServlet.
       * @param service_locator_client Initializes the ServiceLocatorClient.
       * @param entitlements The list of available entitlements.
       * @param data_store Initializes the AdministrationDataStore.
       * @param time_client Initializes the TimeClient.
       */
      template<typename SF, typename DF, typename RF>
      AdministrationServlet(SF&& service_locator_client,
        MarketDataService::EntitlementDatabase entitlements, DF&& data_store,
        RF&& time_client);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      struct RiskStateEntry {
        RiskService::RiskState m_risk_state;
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
      Beam::GetOptionalLocalPtr<S> m_service_locator_client;
      MarketDataService::EntitlementDatabase m_entitlements;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      Beam::GetOptionalLocalPtr<R> m_time_client;
      Beam::ServiceLocator::DirectoryEntry m_administrators_root;
      Beam::ServiceLocator::DirectoryEntry m_services_root;
      Beam::ServiceLocator::DirectoryEntry m_trading_groups_root;
      SyncAccountToSubscribers m_risk_parameters_subscribers;
      SyncRiskStateEntries m_risk_state_entries;
      std::atomic_int m_last_modification_request_id;
      std::atomic_int m_last_message_id;
      Beam::IO::OpenState m_open_state;

      AccountRoles load_account_roles(
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles load_account_roles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);
      bool check_administrator(
        const Beam::ServiceLocator::DirectoryEntry& account);
      bool check_read_permission(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        load_managed_trading_groups(
          const Beam::ServiceLocator::DirectoryEntry& account);
      TradingGroup load_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& directory);
      std::vector<Beam::ServiceLocator::DirectoryEntry> load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void grant_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& admin_account,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements);
      void update_risk_parameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& parameters);
      void ensure_modification_read_permission(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id id);
      AccountModificationRequest make_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& session_account,
        const Beam::ServiceLocator::DirectoryEntry& submission_account,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountRoles& roles, AccountModificationRequest::Type type);
      void store_modification_request(const AccountModificationRequest& request,
        const Message& comment, const AccountRoles& roles);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        on_load_accounts_by_roles(
          ServiceProtocolClient& client, AccountRoles roles);
      Beam::ServiceLocator::DirectoryEntry on_load_administrators_root_entry(
        ServiceProtocolClient& client);
      Beam::ServiceLocator::DirectoryEntry on_load_services_root_entry(
        ServiceProtocolClient& client);
      Beam::ServiceLocator::DirectoryEntry on_load_trading_groups_root_entry(
        ServiceProtocolClient& client);
      bool on_check_administrator_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles on_load_account_roles_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles on_load_supervised_account_roles_request(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);
      Beam::ServiceLocator::DirectoryEntry on_load_parent_trading_group_request(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountIdentity on_load_account_identity_request(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void on_store_account_identity_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);
      TradingGroup on_load_trading_group_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directory);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        on_load_administrators_request(ServiceProtocolClient& client);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        on_load_services_request(ServiceProtocolClient& client);
      MarketDataService::EntitlementDatabase on_load_entitlements_request(
        ServiceProtocolClient& client);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        on_load_account_entitlements_request(ServiceProtocolClient& client,
          const Beam::ServiceLocator::DirectoryEntry& account);
      void on_store_entitlements_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
          entitlements);
      RiskService::RiskParameters on_monitor_risk_parameters_request(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void on_store_risk_parameters_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& parameters);
      RiskService::RiskState on_monitor_risk_state_request(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account);
      void on_store_risk_state_request(Beam::Services::RequestToken<
        ServiceProtocolClient, StoreRiskStateService>& request,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& risk_state);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        on_load_managed_trading_groups_request(ServiceProtocolClient& client,
          const Beam::ServiceLocator::DirectoryEntry& account);
      AccountModificationRequest on_load_account_modification_request(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      std::vector<AccountModificationRequest::Id>
        on_load_account_modification_request_ids(ServiceProtocolClient& client,
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      std::vector<AccountModificationRequest::Id>
        on_load_managed_account_modification_request_ids(
          ServiceProtocolClient& client,
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      EntitlementModification on_load_entitlement_modification(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      AccountModificationRequest on_submit_entitlement_modification_request(
        ServiceProtocolClient& client,
        Beam::ServiceLocator::DirectoryEntry account,
        const EntitlementModification& modification, Message comment);
      RiskModification on_load_risk_modification(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      AccountModificationRequest on_submit_risk_modification_request(
        ServiceProtocolClient& client,
        Beam::ServiceLocator::DirectoryEntry account,
        const RiskModification& modification, Message comment);
      AccountModificationRequest::Update
        on_load_account_modification_request_status(
          ServiceProtocolClient& client, AccountModificationRequest::Id id);
      AccountModificationRequest::Update
        on_approve_account_modification_request(ServiceProtocolClient& client,
          AccountModificationRequest::Id id, Message comment);
      AccountModificationRequest::Update on_reject_account_modification_request(
        ServiceProtocolClient& client, AccountModificationRequest::Id id,
        Message comment);
      Message on_load_message(ServiceProtocolClient& client, Message::Id id);
      std::vector<Message::Id> on_load_message_ids(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      Message on_send_account_modification_request_message(
        ServiceProtocolClient& client, AccountModificationRequest::Id id,
        Message message);
  };

  template<typename S, typename D, typename R>
  struct MetaAdministrationServlet {
    using Session = AdministrationSession;
    template<typename C>
    struct apply {
      using type = AdministrationServlet<C, S, D, R>;
    };
  };

  template<typename C, typename S, typename D, typename R>
  template<typename SF, typename DF, typename RF>
  AdministrationServlet<C, S, D, R>::AdministrationServlet(
      SF&& service_locator_client,
      MarketDataService::EntitlementDatabase entitlements, DF&& data_store,
      RF&& time_client)
      : m_service_locator_client(std::forward<SF>(service_locator_client)),
        m_entitlements(std::move(entitlements)),
        m_data_store(std::forward<DF>(data_store)),
        m_time_client(std::forward<RF>(time_client)) {
    try {
      auto request_ids =
        m_data_store->load_account_modification_request_ids(-1, 1);
      if(request_ids.empty()) {
        m_last_modification_request_id = 0;
      } else {
        m_last_modification_request_id = request_ids.back();
      }
      m_last_message_id = m_data_store->load_last_message_id();
      m_administrators_root = Beam::ServiceLocator::LoadOrCreateDirectory(
        *m_service_locator_client, "administrators",
        Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
      m_services_root = Beam::ServiceLocator::LoadOrCreateDirectory(
        *m_service_locator_client, "services",
        Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
      m_trading_groups_root = Beam::ServiceLocator::LoadOrCreateDirectory(
        *m_service_locator_client, "trading_groups",
        Beam::ServiceLocator::DirectoryEntry::GetStarDirectory());
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterAdministrationServices(Store(slots));
    RegisterAdministrationMessages(Store(slots));
    LoadAccountsByRolesService::AddSlot(Store(slots),
      std::bind_front(&AdministrationServlet::on_load_accounts_by_roles, this));
    LoadAdministratorsRootEntryService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_administrators_root_entry, this));
    LoadServicesRootEntryService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_services_root_entry, this));
    LoadTradingGroupsRootEntryService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_trading_groups_root_entry, this));
    CheckAdministratorService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_check_administrator_request, this));
    LoadAccountRolesService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_account_roles_request, this));
    LoadSupervisedAccountRolesService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_supervised_account_roles_request, this));
    LoadParentTradingGroupService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_parent_trading_group_request, this));
    LoadAccountIdentityService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_account_identity_request, this));
    StoreAccountIdentityService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_store_account_identity_request, this));
    LoadTradingGroupService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_trading_group_request, this));
    LoadAdministratorsService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_administrators_request, this));
    LoadServicesService::AddSlot(Store(slots),
      std::bind_front(&AdministrationServlet::on_load_services_request, this));
    LoadEntitlementsService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_entitlements_request, this));
    LoadAccountEntitlementsService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_account_entitlements_request, this));
    StoreEntitlementsService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_store_entitlements_request, this));
    MonitorRiskParametersService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_monitor_risk_parameters_request, this));
    StoreRiskParametersService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_store_risk_parameters_request, this));
    MonitorRiskStateService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_monitor_risk_state_request, this));
    StoreRiskStateService::AddRequestSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_store_risk_state_request, this));
    LoadManagedTradingGroupsService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_managed_trading_groups_request, this));
    LoadAccountModificationRequestService::AddSlot(
      Store(slots), std::bind_front(
        &AdministrationServlet::on_load_account_modification_request, this));
    LoadAccountModificationRequestIdsService::AddSlot(
      Store(slots), std::bind_front(
        &AdministrationServlet::on_load_account_modification_request_ids,
        this));
    LoadManagedAccountModificationRequestIdsService::AddSlot(
      Store(slots), std::bind_front(&AdministrationServlet::
        on_load_managed_account_modification_request_ids, this));
    LoadEntitlementModificationService::AddSlot(Store(slots), std::bind_front(
      &AdministrationServlet::on_load_entitlement_modification, this));
    SubmitEntitlementModificationRequestService::AddSlot(
      Store(slots), std::bind_front(
        &AdministrationServlet::on_submit_entitlement_modification_request,
        this));
    LoadRiskModificationService::AddSlot(Store(slots),
      std::bind_front(&AdministrationServlet::on_load_risk_modification, this));
    SubmitRiskModificationRequestService::AddSlot(Store(slots), std::bind_front(
        &AdministrationServlet::on_submit_risk_modification_request, this));
    LoadAccountModificationRequestStatusService::AddSlot(Store(slots),
      std::bind_front(
        &AdministrationServlet::on_load_account_modification_request_status,
        this));
    ApproveAccountModificationRequestService::AddSlot(
      Store(slots), std::bind_front(
        &AdministrationServlet::on_approve_account_modification_request, this));
    RejectAccountModificationRequestService::AddSlot(
      Store(slots), std::bind_front(
        &AdministrationServlet::on_reject_account_modification_request, this));
    LoadMessageService::AddSlot(Store(slots),
      std::bind_front(&AdministrationServlet::on_load_message, this));
    LoadMessageIdsService::AddSlot(Store(slots),
      std::bind_front(&AdministrationServlet::on_load_message_ids, this));
    SendAccountModificationRequestMessageService::AddSlot(Store(slots),
      std::bind_front(
        &AdministrationServlet::on_send_account_modification_request_message,
        this));
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::HandleClientClosed(
      ServiceProtocolClient& client) {
    Beam::Threading::With(m_risk_parameters_subscribers,
      [&] (auto& risk_parameters_subscribers) {
        for(auto& subscribers :
            risk_parameters_subscribers | std::views::values) {
          Beam::Threading::With(subscribers, [&] (auto& subscribers) {
            Beam::RemoveAll(subscribers, &client);
          });
        }
      });
    Beam::Threading::With(m_risk_state_entries, [&] (auto& risk_state_entries) {
      for(auto& entry : risk_state_entries | std::views::values) {
        Beam::RemoveAll(entry.m_subscribers, &client);
      }
    });
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::Close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_data_store->close();
    m_open_state.Close();
  }

  template<typename C, typename S, typename D, typename R>
  AccountRoles AdministrationServlet<C, S, D, R>::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto roles = AccountRoles();
    auto parents = m_service_locator_client->LoadParents(account);
    auto trading_groups =
      m_service_locator_client->LoadChildren(m_trading_groups_root);
    for(auto& parent : parents) {
      if(parent == m_administrators_root) {
        roles.Set(AccountRole::ADMINISTRATOR);
      } else if(parent == m_services_root) {
        roles.Set(AccountRole::SERVICE);
      } else if(
          !roles.Test(AccountRole::TRADER) && parent.m_name == "traders") {
        auto entry_parents = m_service_locator_client->LoadParents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::find(trading_groups.begin(), trading_groups.end(),
              entry_parent) != trading_groups.end()) {
            roles.Set(AccountRole::TRADER);
            break;
          }
        }
      } else if(
          !roles.Test(AccountRole::MANAGER) && parent.m_name == "managers") {
        auto entry_parents = m_service_locator_client->LoadParents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::find(trading_groups.begin(), trading_groups.end(),
              entry_parent) != trading_groups.end()) {
            roles.Set(AccountRole::MANAGER);
            break;
          }
        }
      }
    }
    return roles;
  }

  template<typename C, typename S, typename D, typename R>
  AccountRoles AdministrationServlet<C, S, D, R>::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    if(parent == child) {
      return load_account_roles(child);
    }
    auto roles = AccountRoles();
    if(check_administrator(parent)) {
      roles.Set(AccountRole::ADMINISTRATOR);
    }
    auto trading_group_entries = load_managed_trading_groups(parent);
    auto parents = m_service_locator_client->LoadParents(child);
    for(auto& trading_group_entry : trading_group_entries) {
      auto managers_group = m_service_locator_client->LoadDirectoryEntry(
        trading_group_entry, "managers");
      if(std::find(parents.begin(), parents.end(), managers_group) !=
          parents.end()) {
        roles.Set(AccountRole::MANAGER);
        break;
      }
      auto traders_group = m_service_locator_client->LoadDirectoryEntry(
        trading_group_entry, "traders");
      if(std::find(parents.begin(), parents.end(), traders_group) !=
          parents.end()) {
        roles.Set(AccountRole::MANAGER);
        break;
      }
    }
    return roles;
  }

  template<typename C, typename S, typename D, typename R>
  bool AdministrationServlet<C, S, D, R>::check_administrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_service_locator_client->LoadParents(account);
    auto is_administrator = std::find(parents.begin(), parents.end(),
      m_administrators_root) != parents.end();
    return is_administrator;
  }

  template<typename C, typename S, typename D, typename R>
  bool AdministrationServlet<C, S, D, R>::check_read_permission(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    if(parent == child) {
      return true;
    }
    if(check_administrator(parent)) {
      return true;
    }
    auto trading_groups = load_managed_trading_groups(parent);
    auto parent_groups = m_service_locator_client->LoadParents(child);
    for(auto& trading_group : trading_groups) {
      auto traders =
        m_service_locator_client->LoadDirectoryEntry(trading_group, "traders");
      if(std::find(parent_groups.begin(), parent_groups.end(), traders) !=
          parent_groups.end()) {
        return true;
      }
      auto managers =
        m_service_locator_client->LoadDirectoryEntry(trading_group, "managers");
      if(std::find(parent_groups.begin(), parent_groups.end(), managers) !=
          parent_groups.end()) {
        return true;
      }
    }
    return false;
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::load_managed_trading_groups(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_service_locator_client->LoadParents(account);
    auto trading_groups =
      m_service_locator_client->LoadChildren(m_trading_groups_root);
    for(auto& parent : parents) {
      if(parent == m_administrators_root) {
        return trading_groups;
      }
    }
    auto result = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    for(auto& parent : parents) {
      if(parent.m_name == "managers") {
        auto entry_parents = m_service_locator_client->LoadParents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::find(trading_groups.begin(), trading_groups.end(),
              entry_parent) != trading_groups.end()) {
            result.push_back(entry_parent);
            break;
          }
        }
      }
    }
    return result;
  }

  template<typename C, typename S, typename D, typename R>
  TradingGroup AdministrationServlet<C, S, D, R>::load_trading_group(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto managers_directory =
      m_service_locator_client->LoadDirectoryEntry(directory, "managers");
    auto traders_directory =
      m_service_locator_client->LoadDirectoryEntry(directory, "traders");
    auto managers = m_service_locator_client->LoadChildren(managers_directory);
    auto traders = m_service_locator_client->LoadChildren(traders_directory);
    auto trading_group = TradingGroup(directory, std::move(managers_directory),
      std::move(managers), std::move(traders_directory), std::move(traders));
    return trading_group;
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto parents = m_service_locator_client->LoadParents(account);
    auto result = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    for(auto& available_entitlement : m_entitlements.GetEntries()) {
      auto entry_iterator = std::find(
        parents.begin(), parents.end(), available_entitlement.m_groupEntry);
      if(entry_iterator != parents.end()) {
        result.push_back(*entry_iterator);
      }
    }
    return result;
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::grant_entitlements(
      const Beam::ServiceLocator::DirectoryEntry& admin_account,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto existing_entitlements = load_entitlements(account);
    auto entitlement_set =
      std::unordered_set(entitlements.begin(), entitlements.end());
    for(auto& entitlement : m_entitlements.GetEntries()) {
      auto& entry = entitlement.m_groupEntry;
      if(entitlement_set.find(entry) != entitlement_set.end()) {
        if(std::find(existing_entitlements.begin(), existing_entitlements.end(),
            entry) == existing_entitlements.end()) {
          m_service_locator_client->Associate(account, entry);
          auto ss = std::stringstream();
          ss << boost::posix_time::to_simple_string(m_time_client->GetTime()) <<
            ": " << admin_account.m_name << " grants entitlement \"" <<
            entitlement.m_name << "\"" << " to " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      } else {
        if(std::find(existing_entitlements.begin(), existing_entitlements.end(),
            entry) != existing_entitlements.end()) {
          m_service_locator_client->Detach(account, entry);
          auto ss = std::stringstream();
          ss << boost::posix_time::to_simple_string(m_time_client->GetTime()) <<
            ": " << admin_account.m_name << " revokes entitlement \"" <<
            entitlement.m_name << "\"" << " from " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      }
    }
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::update_risk_parameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& parameters) {
    auto& subscribers = Beam::Threading::With(m_risk_parameters_subscribers,
      [&] (auto& risk_parameters_subscribers) -> decltype(auto) {
        return risk_parameters_subscribers[account];
      });
    m_data_store->with_transaction([&] {
      m_data_store->store(account, parameters);
      Beam::Threading::With(subscribers, [&] (auto& subscribers) {
        for(auto& subscriber : subscribers) {
          Beam::Services::SendRecordMessage<RiskParametersMessage>(
            *subscriber, account, parameters);
        }
      });
    });
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::ensure_modification_read_permission(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id id) {
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(account, request.get_account())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
  }

  template<typename C, typename S, typename D, typename R>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      make_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& session_account,
        const Beam::ServiceLocator::DirectoryEntry& submission_account,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountRoles& roles, AccountModificationRequest::Type type) {
    if(!check_read_permission(session_account, submission_account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto request_id = ++m_last_modification_request_id;
    auto timestamp = m_time_client->GetTime();
    return AccountModificationRequest(
      request_id, type, account, submission_account, timestamp);
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::store_modification_request(
      const AccountModificationRequest& request, const Message& comment,
      const AccountRoles& roles) {
    if(comment.get_bodies().size() > 1 ||
        !comment.get_body().m_message.empty()) {
      auto message = Message(++m_last_message_id,
        request.get_submission_account(), request.get_timestamp(),
        comment.get_bodies());
      m_data_store->store(request.get_id(), message);
    }
    auto status = [&] {
      if(roles.Test(AccountRole::ADMINISTRATOR)) {
        return AccountModificationRequest::Status::GRANTED;
      } else if(roles.Test(AccountRole::MANAGER)) {
        return AccountModificationRequest::Status::REVIEWED;
      }
      return AccountModificationRequest::Status::PENDING;
    }();
    auto update = AccountModificationRequest::Update(
      status, request.get_submission_account(), 0, request.get_timestamp());
    m_data_store->store(request.get_id(), update);
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_accounts_by_roles(
        ServiceProtocolClient& client, AccountRoles roles) {
    auto& session = client.GetSession();
    auto accounts = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    if(!check_administrator(session.GetAccount())) {
      return accounts;
    }
    if(roles.Test(AccountRole::ADMINISTRATOR)) {
      auto administrators =
        m_service_locator_client->LoadChildren(m_administrators_root);
      for(auto& administrator : administrators) {
        if(administrator.m_type ==
            Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT) {
          accounts.push_back(std::move(administrator));
        }
      }
    }
    if(roles.Test(AccountRole::MANAGER)) {
      auto trading_groups =
        m_service_locator_client->LoadChildren(m_trading_groups_root);
      for(auto& trading_group : trading_groups) {
        auto managers_directory =
          m_service_locator_client->LoadDirectoryEntry(
            trading_group, "managers");
        auto managers =
          m_service_locator_client->LoadChildren(managers_directory);
        for(auto& manager : managers) {
          if(manager.m_type ==
              Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT) {
            if(std::find(accounts.begin(), accounts.end(), manager) ==
                accounts.end()) {
              accounts.push_back(std::move(manager));
            }
          }
        }
      }
    }
    if(roles.Test(AccountRole::SERVICE)) {
      auto service_accounts =
        m_service_locator_client->LoadChildren(m_services_root);
      for(auto& service_account : service_accounts) {
        if(service_account.m_type ==
            Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT) {
          if(std::find(accounts.begin(), accounts.end(), service_account) ==
              accounts.end()) {
            accounts.push_back(std::move(service_account));
          }
        }
      }
    }
    if(roles.Test(AccountRole::TRADER)) {
      auto trading_groups =
        m_service_locator_client->LoadChildren(m_trading_groups_root);
      for(auto& trading_group : trading_groups) {
        auto traders_directory = m_service_locator_client->LoadDirectoryEntry(
          trading_group, "traders");
        auto traders =
          m_service_locator_client->LoadChildren(traders_directory);
        for(auto& trader : traders) {
          if(trader.m_type ==
              Beam::ServiceLocator::DirectoryEntry::Type::ACCOUNT) {
            if(std::find(accounts.begin(), accounts.end(), trader) ==
                accounts.end()) {
              accounts.push_back(std::move(trader));
            }
          }
        }
      }
    }
    return accounts;
  }

  template<typename C, typename S, typename D, typename R>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_administrators_root_entry(ServiceProtocolClient& client) {
    return m_administrators_root;
  }

  template<typename C, typename S, typename D, typename R>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_services_root_entry(ServiceProtocolClient& client) {
    return m_services_root;
  }

  template<typename C, typename S, typename D, typename R>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_trading_groups_root_entry(ServiceProtocolClient& client) {
    return m_trading_groups_root;
  }

  template<typename C, typename S, typename D, typename R>
  bool AdministrationServlet<C, S, D, R>::on_check_administrator_request(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return check_administrator(account);
  }

  template<typename C, typename S, typename D, typename R>
  AccountRoles AdministrationServlet<C, S, D, R>::on_load_account_roles_request(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return load_account_roles(account);
  }

  template<typename C, typename S, typename D, typename R>
  AccountRoles AdministrationServlet<C, S, D, R>::
      on_load_supervised_account_roles_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), parent) ||
        !check_read_permission(session.GetAccount(), child)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return load_account_roles(parent, child);
  }

  template<typename C, typename S, typename D, typename R>
  Beam::ServiceLocator::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_parent_trading_group_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto parents = m_service_locator_client->LoadParents(account);
    auto trading_groups =
      m_service_locator_client->LoadChildren(m_trading_groups_root);
    for(auto& parent : parents) {
      if(parent.m_name == "traders") {
        auto entry_parents = m_service_locator_client->LoadParents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::find(trading_groups.begin(), trading_groups.end(),
              entry_parent) != trading_groups.end()) {
            return entry_parent;
          }
        }
      }
    }
    return {};
  }

  template<typename C, typename S, typename D, typename R>
  AccountIdentity AdministrationServlet<C, S, D, R>::
      on_load_account_identity_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto identity = m_data_store->with_transaction([&] {
      return m_data_store->load_identity(account);
    });
    identity.m_last_login_time =
      m_service_locator_client->LoadLastLoginTime(account);
    identity.m_registration_time =
      m_service_locator_client->LoadRegistrationTime(account);
    return identity;
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::on_store_account_identity_request(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto& session = client.GetSession();
    if(!check_administrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto account_entry =
      m_service_locator_client->LoadDirectoryEntry(account.m_id);
    m_data_store->with_transaction([&] {
      m_data_store->store(account_entry, identity);
    });
  }

  template<typename C, typename S, typename D, typename R>
  TradingGroup AdministrationServlet<C, S, D, R>::on_load_trading_group_request(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto& session = client.GetSession();
    auto proper_directory =
      m_service_locator_client->LoadDirectoryEntry(directory.m_id);
    if(!check_administrator(session.GetAccount())) {
      auto managed_groups = load_managed_trading_groups(session.GetAccount());
      if(std::find(managed_groups.begin(), managed_groups.end(),
          proper_directory) == managed_groups.end()) {
        throw Beam::Services::ServiceRequestException(
          "Insufficient permissions.");
      }
    }
    return load_trading_group(proper_directory);
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_administrators_request(
        ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(!check_administrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return m_service_locator_client->LoadChildren(m_administrators_root);
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_services_request(
        ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(!check_administrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return m_service_locator_client->LoadChildren(m_services_root);
  }

  template<typename C, typename S, typename D, typename R>
  MarketDataService::EntitlementDatabase AdministrationServlet<C, S, D, R>::
      on_load_entitlements_request(ServiceProtocolClient& client) {
    return m_entitlements;
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_account_entitlements_request(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return load_entitlements(account);
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::on_store_entitlements_request(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto& session = client.GetSession();
    if(!check_administrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    grant_entitlements(session.GetAccount(), account, entitlements);
  }

  template<typename C, typename S, typename D, typename R>
  RiskService::RiskParameters AdministrationServlet<C, S, D, R>::
      on_monitor_risk_parameters_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto& subscribers = Beam::Threading::With(m_risk_parameters_subscribers,
      [&] (auto& risk_parameters_subscribers) -> decltype(auto) {
        return risk_parameters_subscribers[account];
      });
    Beam::Threading::With(subscribers, [&] (auto& subscribers) {
      if(std::find(subscribers.begin(), subscribers.end(), &client) ==
          subscribers.end()) {
        subscribers.push_back(&client);
      }
    });
    auto parameters =  m_data_store->with_transaction([&] {
      return m_data_store->load_risk_parameters(account);
    });
    return parameters;
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::on_store_risk_parameters_request(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& parameters) {
    auto& session = client.GetSession();
    if(!check_administrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    update_risk_parameters(account, parameters);
  }

  template<typename C, typename S, typename D, typename R>
  RiskService::RiskState AdministrationServlet<C, S, D, R>::
      on_monitor_risk_state_request(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto risk_state = m_data_store->with_transaction([&] {
      return Beam::Threading::With(m_risk_state_entries,
        [&] (auto& risk_state_entries) {
          auto& risk_state_entry = risk_state_entries[account];
          auto risk_state = m_data_store->load_risk_state(account);
          if(std::find(risk_state_entry.m_subscribers.begin(),
              risk_state_entry.m_subscribers.end(), &client) !=
              risk_state_entry.m_subscribers.end()) {
            return risk_state;
          }
          risk_state_entry.m_subscribers.push_back(&client);
          return risk_state;
        });
    });
    return risk_state;
  }

  template<typename C, typename S, typename D, typename R>
  void AdministrationServlet<C, S, D, R>::on_store_risk_state_request(
      Beam::Services::RequestToken<ServiceProtocolClient,
      StoreRiskStateService>& request,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& risk_state) {
    auto& session = request.GetSession();
    if(!check_administrator(session.GetAccount())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    m_data_store->with_transaction([&] {
      m_data_store->store(account, risk_state);
      request.SetResult();
      Beam::Threading::With(m_risk_state_entries,
        [&] (auto& risk_state_entries) {
          auto i = risk_state_entries.find(account);
          if(i == risk_state_entries.end()) {
            return;
          }
          auto& risk_state_entry = i->second;
          risk_state_entry.m_risk_state = risk_state;
          for(auto& subscriber : risk_state_entry.m_subscribers) {
            Beam::Services::SendRecordMessage<RiskStateMessage>(
              *subscriber, account, risk_state);
          }
        });
    });
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_managed_trading_groups_request(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return load_managed_trading_groups(account);
  }

  template<typename C, typename S, typename D, typename R>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      on_load_account_modification_request(ServiceProtocolClient& client,
        AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(session.GetAccount(), request.get_account())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return request;
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<AccountModificationRequest::Id>
      AdministrationServlet<C, S, D, R>::
        on_load_account_modification_request_ids(ServiceProtocolClient& client,
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto ids = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request_ids(
        account, start_id, max_count);
    });
    return ids;
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<AccountModificationRequest::Id>
      AdministrationServlet<C, S, D, R>::
        on_load_managed_account_modification_request_ids(
          ServiceProtocolClient& client,
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    auto& session = client.GetSession();
    if(!check_read_permission(session.GetAccount(), account)) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    if(check_administrator(session.GetAccount())) {
      auto ids = m_data_store->with_transaction([&] {
        return m_data_store->load_account_modification_request_ids(
          start_id, max_count);
      });
      return ids;
    }
    auto trading_group_entries = load_managed_trading_groups(account);
    auto accounts = std::vector<Beam::ServiceLocator::DirectoryEntry>();
    for(auto& trading_group_entry : trading_group_entries) {
      auto trading_group = load_trading_group(trading_group_entry);
      std::move(trading_group.get_managers().begin(),
        trading_group.get_managers().end(), std::back_inserter(accounts));
      std::move(trading_group.get_traders().begin(),
        trading_group.get_traders().end(), std::back_inserter(accounts));
    }
    std::sort(accounts.begin(), accounts.end());
    auto ids = std::vector<AccountModificationRequest::Id>();
    for(auto i = std::size_t(0); i != accounts.size(); ++i) {
      if(i != 0 && accounts[i] == accounts[i - 1]) {
        continue;
      }
      auto account_request_ids = m_data_store->with_transaction([&] {
        return m_data_store->load_account_modification_request_ids(
          accounts[i], start_id, max_count);
      });
      ids.insert(
        ids.end(), account_request_ids.begin(), account_request_ids.end());
    }
    std::sort(ids.begin(), ids.end());
    if(static_cast<int>(ids.size()) > max_count) {
      ids.erase(ids.begin() + max_count, ids.end());
    }
    return ids;
  }

  template<typename C, typename S, typename D, typename R>
  EntitlementModification AdministrationServlet<C, S, D, R>::
      on_load_entitlement_modification(
        ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    ensure_modification_read_permission(session.GetAccount(), id);
    auto modification = m_data_store->with_transaction([&] {
      return m_data_store->load_entitlement_modification(id);
    });
    return modification;
  }

  template<typename C, typename S, typename D, typename R>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      on_submit_entitlement_modification_request(ServiceProtocolClient& client,
        Beam::ServiceLocator::DirectoryEntry account,
        const EntitlementModification& modification, Message comment) {
    auto& session = client.GetSession();
    if(account.m_id == -1) {
      account = session.GetAccount();
    }
    auto roles = load_account_roles(session.GetAccount(), account);
    auto request = make_modification_request(
      session.GetAccount(), session.GetAccount(), account, roles,
      AccountModificationRequest::Type::ENTITLEMENTS);
    for(auto& entitlement : modification.get_entitlements()) {
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
    m_data_store->with_transaction([&] {
      m_data_store->store(request, modification);
      store_modification_request(request, comment, roles);
    });
    if(roles.Test(AccountRole::ADMINISTRATOR)) {
      grant_entitlements(request.get_submission_account(),
        request.get_account(), modification.get_entitlements());
    }
    return request;
  }

  template<typename C, typename S, typename D, typename R>
  RiskModification AdministrationServlet<C, S, D, R>::on_load_risk_modification(
      ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    ensure_modification_read_permission(session.GetAccount(), id);
    auto modification = m_data_store->with_transaction([&] {
      return m_data_store->load_risk_modification(id);
    });
    return modification;
  }

  template<typename C, typename S, typename D, typename R>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      on_submit_risk_modification_request(ServiceProtocolClient& client,
        Beam::ServiceLocator::DirectoryEntry account,
        const RiskModification& modification, Message comment) {
    auto& session = client.GetSession();
    if(account.m_id == -1) {
      account = session.GetAccount();
    }
    auto roles = load_account_roles(session.GetAccount(), account);
    auto request = make_modification_request(session.GetAccount(),
      session.GetAccount(), account, roles,
      AccountModificationRequest::Type::RISK);
    m_data_store->with_transaction([&] {
      m_data_store->store(request, modification);
      store_modification_request(request, comment, roles);
    });
    if(roles.Test(AccountRole::ADMINISTRATOR)) {
      update_risk_parameters(
        request.get_account(), modification.get_parameters());
    }
    return request;
  }

  template<typename C, typename S, typename D, typename R>
  AccountModificationRequest::Update AdministrationServlet<C, S, D, R>::
      on_load_account_modification_request_status(
        ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(session.GetAccount(), request.get_account())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto status = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request_status(id);
    });
    return status;
  }

  template<typename C, typename S, typename D, typename R>
  AccountModificationRequest::Update AdministrationServlet<C, S, D, R>::
      on_approve_account_modification_request(ServiceProtocolClient& client,
        AccountModificationRequest::Id id, Message comment) {
    auto& session = client.GetSession();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    auto roles =
      load_account_roles(session.GetAccount(), request.get_account());
    if(roles.GetBitset().none()) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto timestamp = m_time_client->GetTime();
    if(comment.get_bodies().size() == 1 &&
        comment.get_body().m_message.empty()) {
      comment = Message();
    } else {
      comment = Message(++m_last_message_id, session.GetAccount(), timestamp,
        comment.get_bodies());
    }
    auto update = m_data_store->with_transaction([&] {
      auto update = m_data_store->load_account_modification_request_status(
        request.get_id());
      if(is_terminal(update.m_status)) {
        throw Beam::Services::ServiceRequestException(
          "Request can not be updated.");
      }
      if(comment.get_id() != -1) {
        m_data_store->store(request.get_id(), comment);
      }
      if(roles.Test(AccountRole::ADMINISTRATOR)) {
        update.m_status = AccountModificationRequest::Status::GRANTED;
      } else if(roles.Test(AccountRole::MANAGER)) {
        update.m_status = AccountModificationRequest::Status::REVIEWED;
      }
      update.m_account = session.GetAccount();
      ++update.m_sequence_number;
      update.m_timestamp = timestamp;
      m_data_store->store(request.get_id(), update);
      return update;
    });
    if(update.m_status == AccountModificationRequest::Status::GRANTED) {
      if(request.get_type() ==
          AccountModificationRequest::Type::ENTITLEMENTS) {
        auto modification = m_data_store->with_transaction([&] {
          return m_data_store->load_entitlement_modification(
            request.get_id());
        });
        grant_entitlements(update.m_account, request.get_account(),
          modification.get_entitlements());
      } else if(request.get_type() == AccountModificationRequest::Type::RISK) {
        auto modification = m_data_store->with_transaction([&] {
          return m_data_store->load_risk_modification(request.get_id());
        });
        update_risk_parameters(
          request.get_account(), modification.get_parameters());
      }
    }
    return update;
  }

  template<typename C, typename S, typename D, typename R>
  AccountModificationRequest::Update AdministrationServlet<C, S, D, R>::
      on_reject_account_modification_request(ServiceProtocolClient& client,
        AccountModificationRequest::Id id, Message comment) {
    auto& session = client.GetSession();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    auto roles =
      load_account_roles(session.GetAccount(), request.get_account());
    if(roles.GetBitset().none()) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto timestamp = m_time_client->GetTime();
    if(comment.get_bodies().size() == 1 &&
        comment.get_body().m_message.empty()) {
      comment = Message();
    } else {
      comment = Message(++m_last_message_id, session.GetAccount(), timestamp,
        comment.get_bodies());
    }
    auto update = m_data_store->with_transaction([&] {
      auto update = m_data_store->load_account_modification_request_status(
        request.get_id());
      if(is_terminal(update.m_status)) {
        throw Beam::Services::ServiceRequestException(
          "Request can not be updated.");
      }
      if(comment.get_id() != -1) {
        m_data_store->store(request.get_id(), comment);
      }
      update.m_status = AccountModificationRequest::Status::REJECTED;
      update.m_account = session.GetAccount();
      ++update.m_sequence_number;
      update.m_timestamp = timestamp;
      m_data_store->store(request.get_id(), update);
      return update;
    });
    return update;
  }

  template<typename C, typename S, typename D, typename R>
  Message AdministrationServlet<C, S, D, R>::on_load_message(
      ServiceProtocolClient& client, Message::Id id) {
    return m_data_store->with_transaction([&] {
      return m_data_store->load_message(id);
    });
  }

  template<typename C, typename S, typename D, typename R>
  std::vector<Message::Id> AdministrationServlet<C, S, D, R>::
      on_load_message_ids(ServiceProtocolClient& client,
        AccountModificationRequest::Id id) {
    auto& session = client.GetSession();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(session.GetAccount(), request.get_account())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto message_ids = m_data_store->with_transaction([&] {
      return m_data_store->load_message_ids(id);
    });
    return message_ids;
  }

  template<typename C, typename S, typename D, typename R>
  Message AdministrationServlet<C, S, D, R>::
      on_send_account_modification_request_message(
        ServiceProtocolClient& client, AccountModificationRequest::Id id,
        Message message) {
    auto& session = client.GetSession();
    auto account = message.get_account();
    if(account.m_id == -1) {
      account = session.GetAccount();
    } else if(!check_read_permission(
        session.GetAccount(), message.get_account())) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto timestamp = m_time_client->GetTime();
    message = Message(
      ++m_last_message_id, account, timestamp, message.get_bodies());
    m_data_store->with_transaction([&] {
      m_data_store->store(id, message);
    });
    return message;
  }
}

#endif
