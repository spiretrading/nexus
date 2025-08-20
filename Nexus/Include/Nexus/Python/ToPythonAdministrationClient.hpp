#ifndef NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#define NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/AdministrationService/AdministrationClient.hpp"

namespace Nexus::AdministrationService {

  /**
   * Wraps an AdministrationClient for use with Python.
   * @param <C> The type of AdministrationClient to wrap.
   */
  template<typename C>
  class ToPythonAdministrationClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonAdministrationClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonAdministrationClient, Args...>>
      ToPythonAdministrationClient(Args&&... args);

      ~ToPythonAdministrationClient();

      /** Returns the wrapped client. */
      const Client& get_client() const;

      /** Returns the wrapped client. */
      Client& get_client();

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        load_accounts_by_roles(AccountRoles roles);
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
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);
      TradingGroup load_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& directory);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        load_managed_trading_groups(
          const Beam::ServiceLocator::DirectoryEntry& account);
      std::vector<Beam::ServiceLocator::DirectoryEntry> load_administrators();
      std::vector<Beam::ServiceLocator::DirectoryEntry> load_services();
      MarketDataService::EntitlementDatabase load_entitlements();
      std::vector<Beam::ServiceLocator::DirectoryEntry> load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements);
      const Beam::Publisher<RiskService::RiskParameters>&
        get_risk_parameters_publisher(
          const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& parameters);
      const Beam::Publisher<RiskService::RiskState>& get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& state);
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
      AccountModificationRequest submit(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);
      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);
      AccountModificationRequest submit(
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
      boost::optional<Client> m_client;

      ToPythonAdministrationClient(
        const ToPythonAdministrationClient&) = delete;
      ToPythonAdministrationClient& operator =(
        const ToPythonAdministrationClient&) = delete;
  };

  template<typename Client>
  ToPythonAdministrationClient(Client&&) ->
    ToPythonAdministrationClient<std::remove_reference_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonAdministrationClient<C>::ToPythonAdministrationClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonAdministrationClient<C>::~ToPythonAdministrationClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonAdministrationClient<C>::Client&
      ToPythonAdministrationClient<C>::get_client() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonAdministrationClient<C>::Client&
      ToPythonAdministrationClient<C>::get_client() {
    return *m_client;
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_accounts_by_roles(
        AccountRoles roles) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_accounts_by_roles(roles);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::load_administrators_root_entry() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_administrators_root_entry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::load_services_root_entry() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_services_root_entry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::load_trading_groups_root_entry() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_trading_groups_root_entry();
  }

  template<typename C>
  bool ToPythonAdministrationClient<C>::check_administrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->check_administrator(account);
  }

  template<typename C>
  AccountRoles ToPythonAdministrationClient<C>::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_roles(account);
  }

  template<typename C>
  AccountRoles ToPythonAdministrationClient<C>::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_roles(parent, child);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::load_parent_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_parent_trading_group(account);
  }

  template<typename C>
  AccountIdentity ToPythonAdministrationClient<C>::load_identity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_identity(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto release = Beam::Python::GilRelease();
    m_client->store(account, identity);
  }

  template<typename C>
  TradingGroup ToPythonAdministrationClient<C>::load_trading_group(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_trading_group(directory);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_managed_trading_groups(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_managed_trading_groups(account);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_administrators() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_administrators();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_services() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_services();
  }

  template<typename C>
  MarketDataService::EntitlementDatabase
      ToPythonAdministrationClient<C>::load_entitlements() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_entitlements();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_entitlements(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::store_entitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto release = Beam::Python::GilRelease();
    m_client->store_entitlements(account, entitlements);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskParameters>&
      ToPythonAdministrationClient<C>::get_risk_parameters_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->get_risk_parameters_publisher(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& parameters) {
    auto release = Beam::Python::GilRelease();
    m_client->store(account, parameters);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskState>&
      ToPythonAdministrationClient<C>::get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->get_risk_state_publisher(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& state) {
    auto release = Beam::Python::GilRelease();
    m_client->store(account, state);
  }

  template<typename C>
  AccountModificationRequest
      ToPythonAdministrationClient<C>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_modification_request(id);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationClient<C>::load_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationClient<C>::
        load_managed_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_managed_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<typename C>
  EntitlementModification
      ToPythonAdministrationClient<C>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_entitlement_modification(id);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::submit(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const EntitlementModification& modification, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->submit(account, modification, comment);
  }

  template<typename C>
  RiskModification ToPythonAdministrationClient<C>::load_risk_modification(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_risk_modification(id);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::submit(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->submit(account, modification, comment);
  }

  template<typename C>
  AccountModificationRequest::Update
      ToPythonAdministrationClient<C>::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_modification_request_status(id);
  }

  template<typename C>
  AccountModificationRequest::Update
      ToPythonAdministrationClient<C>::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->approve_account_modification_request(id, comment);
  }

  template<typename C>
  AccountModificationRequest::Update
      ToPythonAdministrationClient<C>::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->reject_account_modification_request(id, comment);
  }

  template<typename C>
  Message ToPythonAdministrationClient<C>::load_message(Message::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_message(id);
  }

  template<typename C>
  std::vector<Message::Id> ToPythonAdministrationClient<C>::load_message_ids(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_message_ids(id);
  }

  template<typename C>
  Message ToPythonAdministrationClient<C>::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    auto release = Beam::Python::GilRelease();
    return m_client->send_account_modification_request_message(id, message);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
