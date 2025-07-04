#ifndef NEXUS_ADMINISTRATION_CLIENT_HPP
#define NEXUS_ADMINISTRATION_CLIENT_HPP
#include <memory>
#include <utility>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Queues/Publisher.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus::AdministrationService {

  /** Provides a generic interface over an arbitrary AdministrationClient. */
  class AdministrationClient {
    public:

      template<typename T, typename... Args>
      explicit AdministrationClient(std::in_place_type_t<T>, Args&&... args);

      template<typename AdministrationClient>
      explicit AdministrationClient(AdministrationClient client);

      explicit AdministrationClient(AdministrationClient* client);

      explicit AdministrationClient(
        const std::shared_ptr<AdministrationClient>& client);

      explicit AdministrationClient(
        const std::unique_ptr<AdministrationClient>& client);

      std::vector<Beam::ServiceLocator::DirectoryEntry> load_accounts_by_roles(
        AccountRoles roles);

      Beam::ServiceLocator::DirectoryEntry load_administrators_root_entry();

      Beam::ServiceLocator::DirectoryEntry load_services_root_entry();

      Beam::ServiceLocator::DirectoryEntry load_trading_groups_root_entry();

      bool check_administrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      AccountRoles load_account_roles(
        const Beam::ServiceLocator::DirectoryEntry& account);

      AccountRoles load_account_roles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);

      Beam::ServiceLocator::DirectoryEntry load_parent_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& account);

      AccountIdentity load_identity(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void store_identity(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);

      TradingGroup load_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& directory);

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        load_managed_trading_groups(const Beam::ServiceLocator::DirectoryEntry&
          account);

      std::vector<Beam::ServiceLocator::DirectoryEntry> load_administrators();

      std::vector<Beam::ServiceLocator::DirectoryEntry> load_services();

      MarketDataService::EntitlementDatabase load_entitlements();

      std::vector<Beam::ServiceLocator::DirectoryEntry> load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void store_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
          entitlements);

      const Beam::Publisher<RiskService::RiskParameters>&
        get_risk_parameters_publisher(
          const Beam::ServiceLocator::DirectoryEntry& account);

      void store_risk_parameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& risk_parameters);

      const Beam::Publisher<RiskService::RiskState>& get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void store_risk_state(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& risk_state);

      AccountModificationRequest load_account_modification_request(
        AccountModificationRequest::Id id);

      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);

      std::vector<AccountModificationRequest::Id>
        load_managed_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);

      EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id);

      AccountModificationRequest submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);

      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);

      AccountModificationRequest submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment);

      AccountModificationRequest::Update
        load_account_modification_request_status(
          AccountModificationRequest::Id id);

      AccountModificationRequest::Update approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment);

      AccountModificationRequest::Update reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment);

      Message load_message(Message::Id id);

      std::vector<Message::Id> load_message_ids(
        AccountModificationRequest::Id id);

      Message send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message);

      void close();

    private:
      struct VirtualAdministrationClient {
        virtual ~VirtualAdministrationClient() = default;
        virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_accounts_by_roles(AccountRoles roles) = 0;
        virtual Beam::ServiceLocator::DirectoryEntry
          load_administrators_root_entry() = 0;
        virtual Beam::ServiceLocator::DirectoryEntry
          load_services_root_entry() = 0;
        virtual Beam::ServiceLocator::DirectoryEntry
          load_trading_groups_root_entry() = 0;
        virtual bool check_administrator(
          const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual AccountRoles load_account_roles(
          const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual AccountRoles load_account_roles(
          const Beam::ServiceLocator::DirectoryEntry& parent,
          const Beam::ServiceLocator::DirectoryEntry& child) = 0;
        virtual Beam::ServiceLocator::DirectoryEntry load_parent_trading_group(
          const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual AccountIdentity load_identity(
          const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual void store_identity(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const AccountIdentity& identity) = 0;
        virtual TradingGroup load_trading_group(
          const Beam::ServiceLocator::DirectoryEntry& directory) = 0;
        virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_managed_trading_groups(
            const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_administrators() = 0;
        virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_services() = 0;
        virtual MarketDataService::EntitlementDatabase load_entitlements() = 0;
        virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_entitlements(
            const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual void store_entitlements(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const std::vector<Beam::ServiceLocator::DirectoryEntry>&
            entitlements) = 0;
        virtual const Beam::Publisher<RiskService::RiskParameters>&
          get_risk_parameters_publisher(
            const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual void store_risk_parameters(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const RiskService::RiskParameters& risk_parameters) = 0;
        virtual const Beam::Publisher<RiskService::RiskState>&
          get_risk_state_publisher(
            const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual void store_risk_state(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const RiskService::RiskState& risk_state) = 0;
        virtual AccountModificationRequest load_account_modification_request(
          AccountModificationRequest::Id id) = 0;
        virtual std::vector<AccountModificationRequest::Id>
          load_account_modification_request_ids(
            const Beam::ServiceLocator::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) = 0;
        virtual std::vector<AccountModificationRequest::Id>
          load_managed_account_modification_request_ids(
            const Beam::ServiceLocator::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) = 0;
        virtual EntitlementModification load_entitlement_modification(
          AccountModificationRequest::Id id) = 0;
        virtual AccountModificationRequest submit_account_modification_request(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const EntitlementModification& modification,
          const Message& comment) = 0;
        virtual RiskModification load_risk_modification(
          AccountModificationRequest::Id id) = 0;
        virtual AccountModificationRequest submit_account_modification_request(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const RiskModification& modification, const Message& comment) = 0;
        virtual AccountModificationRequest::Update
          load_account_modification_request_status(
            AccountModificationRequest::Id id) = 0;
        virtual AccountModificationRequest::Update
          approve_account_modification_request(
            AccountModificationRequest::Id id, const Message& comment) = 0;
        virtual AccountModificationRequest::Update
          reject_account_modification_request(AccountModificationRequest::Id id,
            const Message& comment) = 0;
        virtual Message load_message(Message::Id id) = 0;
        virtual std::vector<Message::Id> load_message_ids(
          AccountModificationRequest::Id id) = 0;
        virtual Message send_account_modification_request_message(
          AccountModificationRequest::Id id, const Message& message) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedAdministrationClient final : VirtualAdministrationClient {
        using AdministrationClient = C;
        Beam::GetOptionalLocalPtr<AdministrationClient> m_client;

        template<typename... Args>
        WrappedAdministrationClient(Args&&... args);
        std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_accounts_by_roles(AccountRoles roles) override;
        Beam::ServiceLocator::DirectoryEntry
          load_administrators_root_entry() override;
        Beam::ServiceLocator::DirectoryEntry
          load_services_root_entry() override;
        Beam::ServiceLocator::DirectoryEntry
          load_trading_groups_root_entry() override;
        bool check_administrator(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        AccountRoles load_account_roles(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        AccountRoles load_account_roles(
          const Beam::ServiceLocator::DirectoryEntry& parent,
          const Beam::ServiceLocator::DirectoryEntry& child) override;
        Beam::ServiceLocator::DirectoryEntry load_parent_trading_group(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        AccountIdentity load_identity(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        void store_identity(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const AccountIdentity& identity) override;
        TradingGroup load_trading_group(
          const Beam::ServiceLocator::DirectoryEntry& directory) override;
        std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_managed_trading_groups(
            const Beam::ServiceLocator::DirectoryEntry& account) override;
        std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_administrators() override;
        std::vector<Beam::ServiceLocator::DirectoryEntry>
          load_services() override;
        MarketDataService::EntitlementDatabase load_entitlements() override;
        std::vector<Beam::ServiceLocator::DirectoryEntry> load_entitlements(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        void store_entitlements(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const std::vector<Beam::ServiceLocator::DirectoryEntry>&
            entitlements) override;
        const Beam::Publisher<RiskService::RiskParameters>&
          get_risk_parameters_publisher(
            const Beam::ServiceLocator::DirectoryEntry& account) override;
        void store_risk_parameters(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const RiskService::RiskParameters& risk_parameters) override;
        const Beam::Publisher<RiskService::RiskState>& get_risk_state_publisher(
          const Beam::ServiceLocator::DirectoryEntry& account) override;
        void store_risk_state(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const RiskService::RiskState& risk_state) override;
        AccountModificationRequest load_account_modification_request(
          AccountModificationRequest::Id id) override;
        std::vector<AccountModificationRequest::Id>
          load_account_modification_request_ids(
            const Beam::ServiceLocator::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) override;
        std::vector<AccountModificationRequest::Id>
          load_managed_account_modification_request_ids(
            const Beam::ServiceLocator::DirectoryEntry& account,
            AccountModificationRequest::Id start_id, int max_count) override;
        EntitlementModification load_entitlement_modification(
          AccountModificationRequest::Id id) override;
        AccountModificationRequest submit_account_modification_request(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const EntitlementModification& modification,
          const Message& comment) override;
        RiskModification load_risk_modification(
          AccountModificationRequest::Id id) override;
        AccountModificationRequest submit_account_modification_request(
          const Beam::ServiceLocator::DirectoryEntry& account,
          const RiskModification& modification,
          const Message& comment) override;
        AccountModificationRequest::Update
          load_account_modification_request_status(
            AccountModificationRequest::Id id) override;
        AccountModificationRequest::Update approve_account_modification_request(
          AccountModificationRequest::Id id, const Message& comment) override;
        AccountModificationRequest::Update reject_account_modification_request(
          AccountModificationRequest::Id id, const Message& comment) override;
        Message load_message(Message::Id id) override;
        std::vector<Message::Id> load_message_ids(
          AccountModificationRequest::Id id) override;
        Message send_account_modification_request_message(
          AccountModificationRequest::Id id, const Message& message) override;
        void close() override;
      };
      std::shared_ptr<VirtualAdministrationClient> m_client;
  };

  template<typename T, typename... Args>
  AdministrationClient::AdministrationClient(
    std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedAdministrationClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename AdministrationClient>
  AdministrationClient::AdministrationClient(AdministrationClient client)
    : AdministrationClient(
        std::in_place_type<AdministrationClient>, std::move(client)) {}

  inline AdministrationClient::AdministrationClient(
    AdministrationClient* client)
    : AdministrationClient(*client) {}

  inline AdministrationClient::AdministrationClient(
    const std::shared_ptr<AdministrationClient>& client)
    : AdministrationClient(*client) {}

  inline AdministrationClient::AdministrationClient(
    const std::unique_ptr<AdministrationClient>& client)
    : AdministrationClient(*client) {}

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::load_accounts_by_roles(AccountRoles roles) {
    return m_client->load_accounts_by_roles(roles);
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::load_administrators_root_entry() {
    return m_client->load_administrators_root_entry();
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::load_services_root_entry() {
    return m_client->load_services_root_entry();
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::load_trading_groups_root_entry() {
    return m_client->load_trading_groups_root_entry();
  }

  inline bool AdministrationClient::check_administrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->check_administrator(account);
  }

  inline AccountRoles AdministrationClient::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_account_roles(account);
  }

  inline AccountRoles AdministrationClient::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return m_client->load_account_roles(parent, child);
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::load_parent_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_parent_trading_group(account);
  }

  inline AccountIdentity AdministrationClient::load_identity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_identity(account);
  }

  inline void AdministrationClient::store_identity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_client->store_identity(account, identity);
  }

  inline TradingGroup AdministrationClient::load_trading_group(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    return m_client->load_trading_group(directory);
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::load_managed_trading_groups(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_managed_trading_groups(account);
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::load_administrators() {
    return m_client->load_administrators();
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::load_services() {
    return m_client->load_services();
  }

  inline MarketDataService::EntitlementDatabase
      AdministrationClient::load_entitlements() {
    return m_client->load_entitlements();
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_entitlements(account);
  }

  inline void AdministrationClient::store_entitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    m_client->store_entitlements(account, entitlements);
  }

  inline const Beam::Publisher<RiskService::RiskParameters>&
      AdministrationClient::get_risk_parameters_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->get_risk_parameters_publisher(account);
  }

  inline void AdministrationClient::store_risk_parameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& risk_parameters) {
    m_client->store_risk_parameters(account, risk_parameters);
  }

  inline const Beam::Publisher<RiskService::RiskState>&
      AdministrationClient::get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->get_risk_state_publisher(account);
  }

  inline void AdministrationClient::store_risk_state(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& risk_state) {
    m_client->store_risk_state(account, risk_state);
  }

  inline AccountModificationRequest AdministrationClient::
      load_account_modification_request(AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request(id);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationClient::load_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationClient::load_managed_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_managed_account_modification_request_ids(
      account, start_id, max_count);
  }

  inline EntitlementModification AdministrationClient::
      load_entitlement_modification(AccountModificationRequest::Id id) {
    return m_client->load_entitlement_modification(id);
  }

  inline AccountModificationRequest AdministrationClient::
      submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return m_client->submit_account_modification_request(
      account, modification, comment);
  }

  inline RiskModification AdministrationClient::load_risk_modification(
      AccountModificationRequest::Id id) {
    return m_client->load_risk_modification(id);
  }

  inline AccountModificationRequest AdministrationClient::
    submit_account_modification_request(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    return m_client->submit_account_modification_request(
      account, modification, comment);
  }

  inline AccountModificationRequest::Update AdministrationClient::
      load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request_status(id);
  }

  inline AccountModificationRequest::Update AdministrationClient::
      approve_account_modification_request(AccountModificationRequest::Id id,
        const Message& comment) {
    return m_client->approve_account_modification_request(id, comment);
  }

  inline AccountModificationRequest::Update AdministrationClient::
      reject_account_modification_request(AccountModificationRequest::Id id,
        const Message& comment) {
    return m_client->reject_account_modification_request(id, comment);
  }

  inline Message AdministrationClient::load_message(Message::Id id) {
    return m_client->load_message(id);
  }

  inline std::vector<Message::Id> AdministrationClient::load_message_ids(
      AccountModificationRequest::Id id) {
    return m_client->load_message_ids(id);
  }

  inline Message AdministrationClient::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    return m_client->send_account_modification_request_message(id, message);
  }

  inline void AdministrationClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  AdministrationClient::WrappedAdministrationClient<C>::
    WrappedAdministrationClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_accounts_by_roles(AccountRoles roles) {
    return m_client->load_accounts_by_roles(roles);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::WrappedAdministrationClient<C>::
        load_administrators_root_entry() {
    return m_client->load_administrators_root_entry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::WrappedAdministrationClient<C>::
        load_services_root_entry() {
    return m_client->load_services_root_entry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::WrappedAdministrationClient<C>::
        load_trading_groups_root_entry() {
    return m_client->load_trading_groups_root_entry();
  }

  template<typename C>
  bool AdministrationClient::WrappedAdministrationClient<C>::
      check_administrator(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->check_administrator(account);
  }

  template<typename C>
  AccountRoles AdministrationClient::WrappedAdministrationClient<C>::
      load_account_roles(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_account_roles(account);
  }

  template<typename C>
  AccountRoles AdministrationClient::WrappedAdministrationClient<C>::
      load_account_roles(const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) {
    return m_client->load_account_roles(parent, child);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient::WrappedAdministrationClient<C>::
        load_parent_trading_group(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_parent_trading_group(account);
  }

  template<typename C>
  AccountIdentity AdministrationClient::WrappedAdministrationClient<C>::
      load_identity(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_identity(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::store_identity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_client->store_identity(account, identity);
  }

  template<typename C>
  TradingGroup AdministrationClient::WrappedAdministrationClient<C>::
      load_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& directory) {
    return m_client->load_trading_group(directory);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_managed_trading_groups(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_managed_trading_groups(account);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_administrators() {
    return m_client->load_administrators();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::load_services() {
    return m_client->load_services();
  }

  template<typename C>
  MarketDataService::EntitlementDatabase
      AdministrationClient::WrappedAdministrationClient<C>::
        load_entitlements() {
    return m_client->load_entitlements();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient::WrappedAdministrationClient<C>::load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->load_entitlements(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::
      store_entitlements(const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    m_client->store_entitlements(account, entitlements);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskParameters>&
      AdministrationClient::WrappedAdministrationClient<C>::
        get_risk_parameters_publisher(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->get_risk_parameters_publisher(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::
      store_risk_parameters(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& risk_parameters) {
    m_client->store_risk_parameters(account, risk_parameters);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskState>&
      AdministrationClient::WrappedAdministrationClient<C>::
        get_risk_state_publisher(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->get_risk_state_publisher(account);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::store_risk_state(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& risk_state) {
    m_client->store_risk_state(account, risk_state);
  }

  template<typename C>
  AccountModificationRequest AdministrationClient::
      WrappedAdministrationClient<C>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request(id);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient::WrappedAdministrationClient<C>::
        load_managed_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    return m_client->load_managed_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename C>
  EntitlementModification AdministrationClient::
      WrappedAdministrationClient<C>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    return m_client->load_entitlement_modification(id);
  }

  template<typename C>
  AccountModificationRequest AdministrationClient::
      WrappedAdministrationClient<C>::submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return m_client->submit_account_modification_request(
      account, modification, comment);
  }

  template<typename C>
  RiskModification AdministrationClient::WrappedAdministrationClient<C>::
      load_risk_modification(AccountModificationRequest::Id id) {
    return m_client->load_risk_modification(id);
  }

  template<typename C>
  AccountModificationRequest AdministrationClient::
      WrappedAdministrationClient<C>::submit_account_modification_request(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    return m_client->submit_account_modification_request(
      account, modification, comment);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClient::
      WrappedAdministrationClient<C>::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_client->load_account_modification_request_status(id);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClient::
      WrappedAdministrationClient<C>::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->approve_account_modification_request(id, comment);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClient::
      WrappedAdministrationClient<C>::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->reject_account_modification_request(id, comment);
  }

  template<typename C>
  Message AdministrationClient::WrappedAdministrationClient<C>::load_message(
      Message::Id id) {
    return m_client->load_message(id);
  }

  template<typename C>
  std::vector<Message::Id> AdministrationClient::
      WrappedAdministrationClient<C>::load_message_ids(
        AccountModificationRequest::Id id) {
    return m_client->load_message_ids(id);
  }

  template<typename C>
  Message AdministrationClient::WrappedAdministrationClient<C>::
      send_account_modification_request_message(AccountModificationRequest::Id id,
        const Message& message) {
    return m_client->send_account_modification_request_message(id, message);
  }

  template<typename C>
  void AdministrationClient::WrappedAdministrationClient<C>::close() {
    m_client->close();
  }
}

#endif
