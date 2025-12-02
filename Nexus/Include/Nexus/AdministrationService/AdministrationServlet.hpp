#ifndef NEXUS_ADMINISTRATION_SERVLET_HPP
#define NEXUS_ADMINISTRATION_SERVLET_HPP
#include <atomic>
#include <iostream>
#include <ranges>
#include <sstream>
#include <unordered_set>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationServices.hpp"
#include "Nexus/AdministrationService/AdministrationSession.hpp"

namespace Nexus {

  /**
   * Provides management and administration services for Nexus accounts.
   * @param <C> The container instantiating this servlet.
   * @param <S> The type of ServiceLocatorClient used to manage accounts.
   * @param <D> The type of AdministrationDataStore to use.
   * @param <R> The type of TimeClient to use.
   */
  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  class AdministrationServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of ServiceLocatorClient used to manage accounts. */
      using ServiceLocatorClient = Beam::dereference_t<S>;

      /** The type of AdministrationDataStore used. */
      using AdministrationDataStore = Beam::dereference_t<D>;

      /** The type of TimeClient used. */
      using TimeClient = Beam::dereference_t<R>;

      /**
       * Constructs an AdministrationServlet.
       * @param service_locator_client Initializes the ServiceLocatorClient.
       * @param entitlements The list of available entitlements.
       * @param data_store Initializes the AdministrationDataStore.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<S> SF, Beam::Initializes<D> DF,
        Beam::Initializes<R> RF>
      AdministrationServlet(SF&& service_locator_client,
        EntitlementDatabase entitlements, DF&& data_store, RF&& time_client);

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_closed(ServiceProtocolClient& client);
      void close();

    private:
      struct RiskStateEntry {
        RiskState m_risk_state;
        std::vector<ServiceProtocolClient*> m_subscribers;
      };
      using RiskStateEntries =
        std::unordered_map<Beam::DirectoryEntry, RiskStateEntry>;
      using SyncRiskStateEntries = Beam::Sync<RiskStateEntries>;
      using RiskParameterSubscribers = std::vector<ServiceProtocolClient*>;
      using SyncRiskParameterSubscribers = Beam::Sync<RiskParameterSubscribers>;
      using AccountToRiskSubscribers =
        std::unordered_map<Beam::DirectoryEntry, SyncRiskParameterSubscribers>;
      using SyncAccountToSubscribers = Beam::Sync<AccountToRiskSubscribers>;
      Beam::local_ptr_t<S> m_service_locator_client;
      EntitlementDatabase m_entitlements;
      Beam::local_ptr_t<D> m_data_store;
      Beam::local_ptr_t<R> m_time_client;
      Beam::DirectoryEntry m_administrators_root;
      Beam::DirectoryEntry m_services_root;
      Beam::DirectoryEntry m_trading_groups_root;
      SyncAccountToSubscribers m_risk_parameters_subscribers;
      SyncRiskStateEntries m_risk_state_entries;
      std::atomic_int m_last_modification_request_id;
      std::atomic_int m_last_message_id;
      Beam::OpenState m_open_state;

      AccountRoles load_account_roles(const Beam::DirectoryEntry& account);
      AccountRoles load_account_roles(
        const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child);
      bool check_administrator(const Beam::DirectoryEntry& account);
      bool check_read_permission(
        const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child);
      std::vector<Beam::DirectoryEntry> load_managed_trading_groups(
        const Beam::DirectoryEntry& account);
      TradingGroup load_trading_group(const Beam::DirectoryEntry& directory);
      std::vector<Beam::DirectoryEntry> load_entitlements(
        const Beam::DirectoryEntry& account);
      void grant_entitlements(const Beam::DirectoryEntry& admin_account,
        const Beam::DirectoryEntry& account,
        const std::vector<Beam::DirectoryEntry>& entitlements);
      void update_risk_parameters(
        const Beam::DirectoryEntry& account, const RiskParameters& parameters);
      void ensure_modification_read_permission(
        const Beam::DirectoryEntry& account, AccountModificationRequest::Id id);
      AccountModificationRequest make_modification_request(
        const Beam::DirectoryEntry& session_account,
        const Beam::DirectoryEntry& submission_account,
        const Beam::DirectoryEntry& account, const AccountRoles& roles,
        AccountModificationRequest::Type type);
      void store_modification_request(const AccountModificationRequest& request,
        const Message& comment, const AccountRoles& roles);
      std::vector<Beam::DirectoryEntry> on_load_accounts_by_roles(
        ServiceProtocolClient& client, AccountRoles roles);
      Beam::DirectoryEntry on_load_administrators_root_entry(
        ServiceProtocolClient& client);
      Beam::DirectoryEntry on_load_services_root_entry(
        ServiceProtocolClient& client);
      Beam::DirectoryEntry on_load_trading_groups_root_entry(
        ServiceProtocolClient& client);
      bool on_check_administrator_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      AccountRoles on_load_account_roles_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      AccountRoles on_load_supervised_account_roles_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& parent,
        const Beam::DirectoryEntry& child);
      Beam::DirectoryEntry on_load_parent_trading_group_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      AccountIdentity on_load_account_identity_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      void on_store_account_identity_request(ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);
      TradingGroup on_load_trading_group_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& directory);
      std::vector<Beam::DirectoryEntry> on_load_administrators_request(
        ServiceProtocolClient& client);
      std::vector<Beam::DirectoryEntry> on_load_services_request(
        ServiceProtocolClient& client);
      EntitlementDatabase on_load_entitlements_request(
        ServiceProtocolClient& client);
      std::vector<Beam::DirectoryEntry> on_load_account_entitlements_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      void on_store_entitlements_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
        const std::vector<Beam::DirectoryEntry>& entitlements);
      RiskParameters on_monitor_risk_parameters_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      void on_store_risk_parameters_request(ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account, const RiskParameters& parameters);
      RiskState on_monitor_risk_state_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      void on_store_risk_state_request(Beam::RequestToken<
        ServiceProtocolClient, StoreRiskStateService>& request,
        const Beam::DirectoryEntry& account, const RiskState& risk_state);
      std::vector<Beam::DirectoryEntry> on_load_managed_trading_groups_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account);
      AccountModificationRequest on_load_account_modification_request(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      std::vector<AccountModificationRequest::Id>
        on_load_account_modification_request_ids(
          ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      std::vector<AccountModificationRequest::Id>
        on_load_managed_account_modification_request_ids(
          ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      EntitlementModification on_load_entitlement_modification(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      AccountModificationRequest on_submit_entitlement_modification_request(
        ServiceProtocolClient& client, Beam::DirectoryEntry account,
        const EntitlementModification& modification, Message comment);
      RiskModification on_load_risk_modification(
        ServiceProtocolClient& client, AccountModificationRequest::Id id);
      AccountModificationRequest on_submit_risk_modification_request(
        ServiceProtocolClient& client, Beam::DirectoryEntry account,
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

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  template<Beam::Initializes<S> SF, Beam::Initializes<D> DF,
    Beam::Initializes<R> RF>
  AdministrationServlet<C, S, D, R>::AdministrationServlet(
      SF&& service_locator_client, EntitlementDatabase entitlements,
      DF&& data_store, RF&& time_client)
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
      m_administrators_root = Beam::load_or_create_directory(
        *m_service_locator_client, "administrators",
        Beam::DirectoryEntry::STAR_DIRECTORY);
      m_services_root = Beam::load_or_create_directory(
        *m_service_locator_client, "services",
        Beam::DirectoryEntry::STAR_DIRECTORY);
      m_trading_groups_root = Beam::load_or_create_directory(
        *m_service_locator_client, "trading_groups",
        Beam::DirectoryEntry::STAR_DIRECTORY);
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    register_administration_services(out(slots));
    register_administration_messages(out(slots));
    LoadAccountsByRolesService::add_slot(out(slots),
      std::bind_front(&AdministrationServlet::on_load_accounts_by_roles, this));
    LoadAdministratorsRootEntryService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_administrators_root_entry, this));
    LoadServicesRootEntryService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_services_root_entry, this));
    LoadTradingGroupsRootEntryService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_trading_groups_root_entry, this));
    CheckAdministratorService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_check_administrator_request, this));
    LoadAccountRolesService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_account_roles_request, this));
    LoadSupervisedAccountRolesService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_supervised_account_roles_request, this));
    LoadParentTradingGroupService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_parent_trading_group_request, this));
    LoadAccountIdentityService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_account_identity_request, this));
    StoreAccountIdentityService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_store_account_identity_request, this));
    LoadTradingGroupService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_trading_group_request, this));
    LoadAdministratorsService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_administrators_request, this));
    LoadServicesService::add_slot(out(slots),
      std::bind_front(&AdministrationServlet::on_load_services_request, this));
    LoadEntitlementsService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_entitlements_request, this));
    LoadAccountEntitlementsService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_account_entitlements_request, this));
    StoreEntitlementsService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_store_entitlements_request, this));
    MonitorRiskParametersService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_monitor_risk_parameters_request, this));
    StoreRiskParametersService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_store_risk_parameters_request, this));
    MonitorRiskStateService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_monitor_risk_state_request, this));
    StoreRiskStateService::add_request_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_store_risk_state_request, this));
    LoadManagedTradingGroupsService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_managed_trading_groups_request, this));
    LoadAccountModificationRequestService::add_slot(
      out(slots), std::bind_front(
        &AdministrationServlet::on_load_account_modification_request, this));
    LoadAccountModificationRequestIdsService::add_slot(
      out(slots), std::bind_front(
        &AdministrationServlet::on_load_account_modification_request_ids,
        this));
    LoadManagedAccountModificationRequestIdsService::add_slot(
      out(slots), std::bind_front(&AdministrationServlet::
        on_load_managed_account_modification_request_ids, this));
    LoadEntitlementModificationService::add_slot(out(slots), std::bind_front(
      &AdministrationServlet::on_load_entitlement_modification, this));
    SubmitEntitlementModificationRequestService::add_slot(
      out(slots), std::bind_front(
        &AdministrationServlet::on_submit_entitlement_modification_request,
        this));
    LoadRiskModificationService::add_slot(out(slots),
      std::bind_front(&AdministrationServlet::on_load_risk_modification, this));
    SubmitRiskModificationRequestService::add_slot(out(slots), std::bind_front(
        &AdministrationServlet::on_submit_risk_modification_request, this));
    LoadAccountModificationRequestStatusService::add_slot(out(slots),
      std::bind_front(
        &AdministrationServlet::on_load_account_modification_request_status,
        this));
    ApproveAccountModificationRequestService::add_slot(
      out(slots), std::bind_front(
        &AdministrationServlet::on_approve_account_modification_request, this));
    RejectAccountModificationRequestService::add_slot(
      out(slots), std::bind_front(
        &AdministrationServlet::on_reject_account_modification_request, this));
    LoadMessageService::add_slot(out(slots),
      std::bind_front(&AdministrationServlet::on_load_message, this));
    LoadMessageIdsService::add_slot(out(slots),
      std::bind_front(&AdministrationServlet::on_load_message_ids, this));
    SendAccountModificationRequestMessageService::add_slot(out(slots),
      std::bind_front(
        &AdministrationServlet::on_send_account_modification_request_message,
        this));
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::handle_closed(
      ServiceProtocolClient& client) {
    Beam::with(m_risk_parameters_subscribers,
      [&] (auto& risk_parameters_subscribers) {
        for(auto& subscribers :
            risk_parameters_subscribers | std::views::values) {
          Beam::with(subscribers, [&] (auto& subscribers) {
            std::erase(subscribers, &client);
          });
        }
      });
    Beam::with(m_risk_state_entries, [&] (auto& risk_state_entries) {
      for(auto& entry : risk_state_entries | std::views::values) {
        std::erase(entry.m_subscribers, &client);
      }
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_data_store->close();
    m_open_state.close();
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountRoles AdministrationServlet<C, S, D, R>::load_account_roles(
      const Beam::DirectoryEntry& account) {
    auto roles = AccountRoles();
    auto parents = m_service_locator_client->load_parents(account);
    auto trading_groups =
      m_service_locator_client->load_children(m_trading_groups_root);
    for(auto& parent : parents) {
      if(parent == m_administrators_root) {
        roles.set(AccountRole::ADMINISTRATOR);
      } else if(parent == m_services_root) {
        roles.set(AccountRole::SERVICE);
      } else if(
          !roles.test(AccountRole::TRADER) && parent.m_name == "traders") {
        auto entry_parents = m_service_locator_client->load_parents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::ranges::contains(trading_groups, entry_parent)) {
            roles.set(AccountRole::TRADER);
            break;
          }
        }
      } else if(
          !roles.test(AccountRole::MANAGER) && parent.m_name == "managers") {
        auto entry_parents = m_service_locator_client->load_parents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::ranges::contains(trading_groups, entry_parent)) {
            roles.set(AccountRole::MANAGER);
            break;
          }
        }
      }
    }
    return roles;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountRoles AdministrationServlet<C, S, D, R>::load_account_roles(
      const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child) {
    if(parent == child) {
      return load_account_roles(child);
    }
    auto roles = AccountRoles();
    if(check_administrator(parent)) {
      roles.set(AccountRole::ADMINISTRATOR);
    }
    auto trading_group_entries = load_managed_trading_groups(parent);
    auto parents = m_service_locator_client->load_parents(child);
    for(auto& trading_group_entry : trading_group_entries) {
      auto managers_group = m_service_locator_client->load_directory_entry(
        trading_group_entry, "managers");
      if(std::ranges::contains(parents, managers_group)) {
        roles.set(AccountRole::MANAGER);
        break;
      }
      auto traders_group = m_service_locator_client->load_directory_entry(
        trading_group_entry, "traders");
      if(std::ranges::contains(parents, traders_group)) {
        roles.set(AccountRole::MANAGER);
        break;
      }
    }
    return roles;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  bool AdministrationServlet<C, S, D, R>::check_administrator(
      const Beam::DirectoryEntry& account) {
    auto parents = m_service_locator_client->load_parents(account);
    return std::ranges::contains(parents, m_administrators_root);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  bool AdministrationServlet<C, S, D, R>::check_read_permission(
      const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child) {
    if(parent == child) {
      return true;
    }
    if(check_administrator(parent)) {
      return true;
    }
    auto trading_groups = load_managed_trading_groups(parent);
    auto parent_groups = m_service_locator_client->load_parents(child);
    for(auto& trading_group : trading_groups) {
      auto traders = m_service_locator_client->load_directory_entry(
        trading_group, "traders");
      if(std::ranges::contains(parent_groups, traders)) {
        return true;
      }
      auto managers = m_service_locator_client->load_directory_entry(
        trading_group, "managers");
      if(std::ranges::contains(parent_groups, managers)) {
        return true;
      }
    }
    return false;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Beam::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::load_managed_trading_groups(
        const Beam::DirectoryEntry& account) {
    auto parents = m_service_locator_client->load_parents(account);
    auto trading_groups =
      m_service_locator_client->load_children(m_trading_groups_root);
    for(auto& parent : parents) {
      if(parent == m_administrators_root) {
        return trading_groups;
      }
    }
    auto result = std::vector<Beam::DirectoryEntry>();
    for(auto& parent : parents) {
      if(parent.m_name == "managers") {
        auto entry_parents = m_service_locator_client->load_parents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::ranges::contains(trading_groups, entry_parent)) {
            result.push_back(entry_parent);
            break;
          }
        }
      }
    }
    return result;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  TradingGroup AdministrationServlet<C, S, D, R>::load_trading_group(
      const Beam::DirectoryEntry& directory) {
    auto managers_directory =
      m_service_locator_client->load_directory_entry(directory, "managers");
    auto traders_directory =
      m_service_locator_client->load_directory_entry(directory, "traders");
    auto managers = m_service_locator_client->load_children(managers_directory);
    auto traders = m_service_locator_client->load_children(traders_directory);
    auto trading_group = TradingGroup(directory, std::move(managers_directory),
      std::move(managers), std::move(traders_directory), std::move(traders));
    return trading_group;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Beam::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::load_entitlements(
        const Beam::DirectoryEntry& account) {
    auto parents = m_service_locator_client->load_parents(account);
    auto result = std::vector<Beam::DirectoryEntry>();
    for(auto& available_entitlement : m_entitlements.get_entries()) {
      if(std::ranges::contains(parents, available_entitlement.m_group_entry)) {
        result.push_back(available_entitlement.m_group_entry);
      }
    }
    return result;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::grant_entitlements(
      const Beam::DirectoryEntry& admin_account,
      const Beam::DirectoryEntry& account,
      const std::vector<Beam::DirectoryEntry>& entitlements) {
    auto existing_entitlements = load_entitlements(account);
    auto entitlement_set =
      std::unordered_set(entitlements.begin(), entitlements.end());
    for(auto& entitlement : m_entitlements.get_entries()) {
      auto& entry = entitlement.m_group_entry;
      if(entitlement_set.contains(entry)) {
        if(!std::ranges::contains(existing_entitlements, entry)) {
          m_service_locator_client->associate(account, entry);
          auto ss = std::stringstream();
          ss <<
            boost::posix_time::to_simple_string(m_time_client->get_time()) <<
            ": " << admin_account.m_name << " grants entitlement \"" <<
            entitlement.m_name << "\"" << " to " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      } else {
        if(std::ranges::contains(existing_entitlements, entry)) {
          m_service_locator_client->detach(account, entry);
          auto ss = std::stringstream();
          ss <<
            boost::posix_time::to_simple_string(m_time_client->get_time()) <<
            ": " << admin_account.m_name << " revokes entitlement \"" <<
            entitlement.m_name << "\"" << " from " << account.m_name << ".\n";
          std::cout << ss.str() << std::flush;
        }
      }
    }
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::update_risk_parameters(
      const Beam::DirectoryEntry& account,
      const RiskParameters& parameters) {
    auto& subscribers = Beam::with(m_risk_parameters_subscribers,
      [&] (auto& risk_parameters_subscribers) -> decltype(auto) {
        return risk_parameters_subscribers[account];
      });
    m_data_store->with_transaction([&] {
      m_data_store->store(account, parameters);
      Beam::with(subscribers, [&] (auto& subscribers) {
        for(auto& subscriber : subscribers) {
          Beam::send_record_message<RiskParametersMessage>(
            *subscriber, account, parameters);
        }
      });
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::ensure_modification_read_permission(
      const Beam::DirectoryEntry& account,
      AccountModificationRequest::Id id) {
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(account, request.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      make_modification_request(const Beam::DirectoryEntry& session_account,
        const Beam::DirectoryEntry& submission_account,
        const Beam::DirectoryEntry& account, const AccountRoles& roles,
        AccountModificationRequest::Type type) {
    if(!check_read_permission(session_account, submission_account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto request_id = ++m_last_modification_request_id;
    auto timestamp = m_time_client->get_time();
    return AccountModificationRequest(
      request_id, type, account, submission_account, timestamp);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
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
      if(roles.test(AccountRole::ADMINISTRATOR)) {
        return AccountModificationRequest::Status::GRANTED;
      } else if(roles.test(AccountRole::MANAGER)) {
        return AccountModificationRequest::Status::REVIEWED;
      }
      return AccountModificationRequest::Status::PENDING;
    }();
    auto update = AccountModificationRequest::Update(
      status, request.get_submission_account(), 0, request.get_timestamp());
    m_data_store->store(request.get_id(), update);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Beam::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_accounts_by_roles(
        ServiceProtocolClient& client, AccountRoles roles) {
    auto& session = client.get_session();
    auto accounts = std::vector<Beam::DirectoryEntry>();
    if(!check_administrator(session.get_account())) {
      return accounts;
    }
    if(roles.test(AccountRole::ADMINISTRATOR)) {
      auto administrators =
        m_service_locator_client->load_children(m_administrators_root);
      for(auto& administrator : administrators) {
        if(administrator.m_type == Beam::DirectoryEntry::Type::ACCOUNT) {
          accounts.push_back(std::move(administrator));
        }
      }
    }
    if(roles.test(AccountRole::MANAGER)) {
      auto trading_groups =
        m_service_locator_client->load_children(m_trading_groups_root);
      for(auto& trading_group : trading_groups) {
        auto managers_directory =
          m_service_locator_client->load_directory_entry(
            trading_group, "managers");
        auto managers =
          m_service_locator_client->load_children(managers_directory);
        for(auto& manager : managers) {
          if(manager.m_type == Beam::DirectoryEntry::Type::ACCOUNT) {
            if(!std::ranges::contains(accounts, manager)) {
              accounts.push_back(std::move(manager));
            }
          }
        }
      }
    }
    if(roles.test(AccountRole::SERVICE)) {
      auto service_accounts =
        m_service_locator_client->load_children(m_services_root);
      for(auto& service_account : service_accounts) {
        if(service_account.m_type == Beam::DirectoryEntry::Type::ACCOUNT) {
          if(!std::ranges::contains(accounts, service_account)) {
            accounts.push_back(std::move(service_account));
          }
        }
      }
    }
    if(roles.test(AccountRole::TRADER)) {
      auto trading_groups =
        m_service_locator_client->load_children(m_trading_groups_root);
      for(auto& trading_group : trading_groups) {
        auto traders_directory = m_service_locator_client->load_directory_entry(
          trading_group, "traders");
        auto traders =
          m_service_locator_client->load_children(traders_directory);
        for(auto& trader : traders) {
          if(trader.m_type == Beam::DirectoryEntry::Type::ACCOUNT) {
            if(!std::ranges::contains(accounts, trader)) {
              accounts.push_back(std::move(trader));
            }
          }
        }
      }
    }
    return accounts;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  Beam::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_administrators_root_entry(ServiceProtocolClient& client) {
    return m_administrators_root;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  Beam::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_services_root_entry(ServiceProtocolClient& client) {
    return m_services_root;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  Beam::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_trading_groups_root_entry(ServiceProtocolClient& client) {
    return m_trading_groups_root;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  bool AdministrationServlet<C, S, D, R>::on_check_administrator_request(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account) {
    return check_administrator(account);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountRoles AdministrationServlet<C, S, D, R>::on_load_account_roles_request(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return load_account_roles(account);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountRoles AdministrationServlet<C, S, D, R>::
      on_load_supervised_account_roles_request(ServiceProtocolClient& client,
        const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), parent) ||
        !check_read_permission(session.get_account(), child)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return load_account_roles(parent, child);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  Beam::DirectoryEntry AdministrationServlet<C, S, D, R>::
      on_load_parent_trading_group_request(ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto parents = m_service_locator_client->load_parents(account);
    auto trading_groups =
      m_service_locator_client->load_children(m_trading_groups_root);
    for(auto& parent : parents) {
      if(parent.m_name == "traders") {
        auto entry_parents = m_service_locator_client->load_parents(parent);
        for(auto& entry_parent : entry_parents) {
          if(std::ranges::contains(trading_groups, entry_parent)) {
            return entry_parent;
          }
        }
      }
    }
    return {};
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountIdentity AdministrationServlet<C, S, D, R>::
      on_load_account_identity_request(ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto identity = m_data_store->with_transaction([&] {
      return m_data_store->load_identity(account);
    });
    identity.m_last_login_time =
      m_service_locator_client->load_last_login_time(account);
    identity.m_registration_time =
      m_service_locator_client->load_registration_time(account);
    return identity;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::on_store_account_identity_request(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto& session = client.get_session();
    if(!check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto account_entry =
      m_service_locator_client->load_directory_entry(account.m_id);
    m_data_store->with_transaction([&] {
      m_data_store->store(account_entry, identity);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  TradingGroup AdministrationServlet<C, S, D, R>::on_load_trading_group_request(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& directory) {
    auto& session = client.get_session();
    auto proper_directory =
      m_service_locator_client->load_directory_entry(directory.m_id);
    if(!check_administrator(session.get_account())) {
      auto managed_groups = load_managed_trading_groups(session.get_account());
      if(!std::ranges::contains(managed_groups, proper_directory)) {
        boost::throw_with_location(
          Beam::ServiceRequestException("Insufficient permissions."));
      }
    }
    return load_trading_group(proper_directory);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Beam::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_administrators_request(
        ServiceProtocolClient& client) {
    auto& session = client.get_session();
    if(!check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_service_locator_client->load_children(m_administrators_root);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Beam::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_services_request(
        ServiceProtocolClient& client) {
    auto& session = client.get_session();
    if(!check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_service_locator_client->load_children(m_services_root);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  EntitlementDatabase AdministrationServlet<C, S, D, R>::
      on_load_entitlements_request(ServiceProtocolClient& client) {
    return m_entitlements;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Beam::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_account_entitlements_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return load_entitlements(account);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::on_store_entitlements_request(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
      const std::vector<Beam::DirectoryEntry>& entitlements) {
    auto& session = client.get_session();
    if(!check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    grant_entitlements(session.get_account(), account, entitlements);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  RiskParameters AdministrationServlet<C, S, D, R>::
      on_monitor_risk_parameters_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto& subscribers = Beam::with(m_risk_parameters_subscribers,
      [&] (auto& risk_parameters_subscribers) -> decltype(auto) {
        return risk_parameters_subscribers[account];
      });
    Beam::with(subscribers, [&] (auto& subscribers) {
      if(!std::ranges::contains(subscribers, &client)) {
        subscribers.push_back(&client);
      }
    });
    return m_data_store->with_transaction([&] {
      return m_data_store->load_risk_parameters(account);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::on_store_risk_parameters_request(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
      const RiskParameters& parameters) {
    auto& session = client.get_session();
    if(!check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    update_risk_parameters(account, parameters);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  RiskState AdministrationServlet<C, S, D, R>::on_monitor_risk_state_request(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_data_store->with_transaction([&] {
      return Beam::with(m_risk_state_entries, [&] (auto& risk_state_entries) {
        auto& risk_state_entry = risk_state_entries[account];
        auto risk_state = m_data_store->load_risk_state(account);
        if(std::ranges::contains(risk_state_entry.m_subscribers, &client)) {
          return risk_state;
        }
        risk_state_entry.m_subscribers.push_back(&client);
        return risk_state;
      });
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  void AdministrationServlet<C, S, D, R>::on_store_risk_state_request(
      Beam::RequestToken<ServiceProtocolClient, StoreRiskStateService>& request,
      const Beam::DirectoryEntry& account, const RiskState& risk_state) {
    auto& session = request.get_session();
    if(!check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    m_data_store->with_transaction([&] {
      m_data_store->store(account, risk_state);
      request.set();
      Beam::with(m_risk_state_entries, [&] (auto& risk_state_entries) {
        auto i = risk_state_entries.find(account);
        if(i == risk_state_entries.end()) {
          return;
        }
        auto& risk_state_entry = i->second;
        risk_state_entry.m_risk_state = risk_state;
        for(auto& subscriber : risk_state_entry.m_subscribers) {
          Beam::send_record_message<RiskStateMessage>(
            *subscriber, account, risk_state);
        }
      });
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Beam::DirectoryEntry>
      AdministrationServlet<C, S, D, R>::on_load_managed_trading_groups_request(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return load_managed_trading_groups(account);
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      on_load_account_modification_request(ServiceProtocolClient& client,
        AccountModificationRequest::Id id) {
    auto& session = client.get_session();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(session.get_account(), request.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return request;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<AccountModificationRequest::Id>
      AdministrationServlet<C, S, D, R>::
        on_load_account_modification_request_ids(ServiceProtocolClient& client,
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request_ids(
        account, start_id, max_count);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<AccountModificationRequest::Id>
      AdministrationServlet<C, S, D, R>::
        on_load_managed_account_modification_request_ids(
          ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    auto& session = client.get_session();
    if(!check_read_permission(session.get_account(), account)) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    if(check_administrator(session.get_account())) {
      return m_data_store->with_transaction([&] {
        return m_data_store->load_account_modification_request_ids(
          start_id, max_count);
      });
    }
    auto trading_group_entries = load_managed_trading_groups(account);
    auto accounts = std::vector<Beam::DirectoryEntry>();
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

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  EntitlementModification AdministrationServlet<C, S, D, R>::
      on_load_entitlement_modification(
        ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.get_session();
    ensure_modification_read_permission(session.get_account(), id);
    return m_data_store->with_transaction([&] {
      return m_data_store->load_entitlement_modification(id);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      on_submit_entitlement_modification_request(ServiceProtocolClient& client,
        Beam::DirectoryEntry account,
        const EntitlementModification& modification, Message comment) {
    auto& session = client.get_session();
    if(account.m_id == -1) {
      account = session.get_account();
    }
    auto roles = load_account_roles(session.get_account(), account);
    auto request = make_modification_request(
      session.get_account(), session.get_account(), account, roles,
      AccountModificationRequest::Type::ENTITLEMENTS);
    for(auto& entitlement : modification.get_entitlements()) {
      if(entitlement.m_type != Beam::DirectoryEntry::Type::DIRECTORY) {
        boost::throw_with_location(
          Beam::ServiceRequestException("Invalid entitlement."));
      }
      auto entries = m_entitlements.get_entries();
      auto is_entitlement_found =
        std::ranges::any_of(entries, [&] (const auto& entry) {
          return entry.m_group_entry == entitlement;
        });
      if(!is_entitlement_found) {
        boost::throw_with_location(
          Beam::ServiceRequestException("Invalid entitlement."));
      }
    }
    m_data_store->with_transaction([&] {
      m_data_store->store(request, modification);
      store_modification_request(request, comment, roles);
    });
    if(roles.test(AccountRole::ADMINISTRATOR)) {
      grant_entitlements(request.get_submission_account(),
        request.get_account(), modification.get_entitlements());
    }
    return request;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  RiskModification AdministrationServlet<C, S, D, R>::on_load_risk_modification(
      ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.get_session();
    ensure_modification_read_permission(session.get_account(), id);
    return m_data_store->with_transaction([&] {
      return m_data_store->load_risk_modification(id);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountModificationRequest AdministrationServlet<C, S, D, R>::
      on_submit_risk_modification_request(ServiceProtocolClient& client,
        Beam::DirectoryEntry account, const RiskModification& modification,
        Message comment) {
    auto& session = client.get_session();
    if(account.m_id == -1) {
      account = session.get_account();
    }
    auto roles = load_account_roles(session.get_account(), account);
    auto request = make_modification_request(
      session.get_account(), session.get_account(), account, roles,
      AccountModificationRequest::Type::RISK);
    m_data_store->with_transaction([&] {
      m_data_store->store(request, modification);
      store_modification_request(request, comment, roles);
    });
    if(roles.test(AccountRole::ADMINISTRATOR)) {
      update_risk_parameters(
        request.get_account(), modification.get_parameters());
    }
    return request;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountModificationRequest::Update AdministrationServlet<C, S, D, R>::
      on_load_account_modification_request_status(
        ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.get_session();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(session.get_account(), request.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request_status(id);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountModificationRequest::Update AdministrationServlet<C, S, D, R>::
      on_approve_account_modification_request(ServiceProtocolClient& client,
        AccountModificationRequest::Id id, Message comment) {
    auto& session = client.get_session();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    auto roles =
      load_account_roles(session.get_account(), request.get_account());
    if(roles.get_bitset().none()) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto timestamp = m_time_client->get_time();
    if(comment.get_bodies().size() == 1 &&
        comment.get_body().m_message.empty()) {
      comment = Message();
    } else {
      comment = Message(++m_last_message_id, session.get_account(), timestamp,
        comment.get_bodies());
    }
    auto update = m_data_store->with_transaction([&] {
      auto update = m_data_store->load_account_modification_request_status(
        request.get_id());
      if(is_terminal(update.m_status)) {
        boost::throw_with_location(
          Beam::ServiceRequestException("Request can not be updated."));
      }
      if(comment.get_id() != -1) {
        m_data_store->store(request.get_id(), comment);
      }
      if(roles.test(AccountRole::ADMINISTRATOR)) {
        update.m_status = AccountModificationRequest::Status::GRANTED;
      } else if(roles.test(AccountRole::MANAGER)) {
        update.m_status = AccountModificationRequest::Status::REVIEWED;
      }
      update.m_account = session.get_account();
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

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  AccountModificationRequest::Update AdministrationServlet<C, S, D, R>::
      on_reject_account_modification_request(ServiceProtocolClient& client,
        AccountModificationRequest::Id id, Message comment) {
    auto& session = client.get_session();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    auto roles =
      load_account_roles(session.get_account(), request.get_account());
    if(roles.get_bitset().none()) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto timestamp = m_time_client->get_time();
    if(comment.get_bodies().size() == 1 &&
        comment.get_body().m_message.empty()) {
      comment = Message();
    } else {
      comment = Message(++m_last_message_id, session.get_account(), timestamp,
        comment.get_bodies());
    }
    auto update = m_data_store->with_transaction([&] {
      auto update = m_data_store->load_account_modification_request_status(
        request.get_id());
      if(is_terminal(update.m_status)) {
        boost::throw_with_location(
          Beam::ServiceRequestException("Request can not be updated."));
      }
      if(comment.get_id() != -1) {
        m_data_store->store(request.get_id(), comment);
      }
      update.m_status = AccountModificationRequest::Status::REJECTED;
      update.m_account = session.get_account();
      ++update.m_sequence_number;
      update.m_timestamp = timestamp;
      m_data_store->store(request.get_id(), update);
      return update;
    });
    return update;
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  Message AdministrationServlet<C, S, D, R>::on_load_message(
      ServiceProtocolClient& client, Message::Id id) {
    return m_data_store->with_transaction([&] {
      return m_data_store->load_message(id);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  std::vector<Message::Id> AdministrationServlet<C, S, D, R>::
      on_load_message_ids(
        ServiceProtocolClient& client, AccountModificationRequest::Id id) {
    auto& session = client.get_session();
    auto request = m_data_store->with_transaction([&] {
      return m_data_store->load_account_modification_request(id);
    });
    if(!check_read_permission(session.get_account(), request.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_data_store->with_transaction([&] {
      return m_data_store->load_message_ids(id);
    });
  }

  template<typename C, typename S, typename D, typename R> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationDataStore<Beam::dereference_t<D>> &&
        Beam::IsTimeClient<Beam::dereference_t<R>>
  Message AdministrationServlet<C, S, D, R>::
      on_send_account_modification_request_message(
        ServiceProtocolClient& client, AccountModificationRequest::Id id,
        Message message) {
    auto& session = client.get_session();
    auto account = message.get_account();
    if(account.m_id == -1) {
      account = session.get_account();
    } else if(!check_read_permission(
        session.get_account(), message.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto timestamp = m_time_client->get_time();
    message = Message(
      ++m_last_message_id, account, timestamp, message.get_bodies());
    m_data_store->with_transaction([&] {
      m_data_store->store(id, message);
    });
    return message;
  }
}

#endif
