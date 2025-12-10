#ifndef NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#define NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"

namespace Nexus {

  /**
   * Wraps an AdministrationClient for use with Python.
   * @param <C> The type of AdministrationClient to wrap.
   */
  template<IsAdministrationClient C>
  class ToPythonAdministrationClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonAdministrationClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonAdministrationClient(Args&&... args);

      ~ToPythonAdministrationClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      std::vector<Beam::DirectoryEntry>
        load_accounts_by_roles(AccountRoles roles);
      Beam::DirectoryEntry load_administrators_root_entry();
      Beam::DirectoryEntry load_services_root_entry();
      Beam::DirectoryEntry load_trading_groups_root_entry();
      bool check_administrator(const Beam::DirectoryEntry& account);
      AccountRoles load_account_roles(const Beam::DirectoryEntry& account);
      AccountRoles load_account_roles(
        const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child);
      Beam::DirectoryEntry load_parent_trading_group(
        const Beam::DirectoryEntry& account);
      AccountIdentity load_identity(const Beam::DirectoryEntry& account);
      void store(
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);
      TradingGroup load_trading_group(const Beam::DirectoryEntry& directory);
      std::vector<Beam::DirectoryEntry>
        load_managed_trading_groups(const Beam::DirectoryEntry& account);
      std::vector<Beam::DirectoryEntry> load_administrators();
      std::vector<Beam::DirectoryEntry> load_services();
      EntitlementDatabase load_entitlements();
      std::vector<Beam::DirectoryEntry> load_entitlements(
        const Beam::DirectoryEntry& account);
      void store_entitlements(const Beam::DirectoryEntry& account,
        const std::vector<Beam::DirectoryEntry>& entitlements);
      const Beam::Publisher<RiskParameters>& get_risk_parameters_publisher(
        const Beam::DirectoryEntry& account);
      void store(
        const Beam::DirectoryEntry& account, const RiskParameters& parameters);
      const Beam::Publisher<RiskState>& get_risk_state_publisher(
        const Beam::DirectoryEntry& account);
      void store(const Beam::DirectoryEntry& account, const RiskState& state);
      AccountModificationRequest load_account_modification_request(
        AccountModificationRequest::Id id);
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      std::vector<AccountModificationRequest::Id>
        load_managed_account_modification_request_ids(
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id);
      AccountModificationRequest submit(const Beam::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);
      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);
      AccountModificationRequest submit(const Beam::DirectoryEntry& account,
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
    ToPythonAdministrationClient<std::remove_cvref_t<Client>>;

  template<IsAdministrationClient C>
  template<typename... Args>
  ToPythonAdministrationClient<C>::ToPythonAdministrationClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsAdministrationClient C>
  ToPythonAdministrationClient<C>::~ToPythonAdministrationClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsAdministrationClient C>
  typename ToPythonAdministrationClient<C>::Client&
      ToPythonAdministrationClient<C>::get() {
    return *m_client;
  }

  template<IsAdministrationClient C>
  const typename ToPythonAdministrationClient<C>::Client&
      ToPythonAdministrationClient<C>::get() const {
    return *m_client;
  }

  template<IsAdministrationClient C>
  std::vector<Beam::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_accounts_by_roles(
        AccountRoles roles) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_accounts_by_roles(roles);
  }

  template<IsAdministrationClient C>
  Beam::DirectoryEntry
      ToPythonAdministrationClient<C>::load_administrators_root_entry() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_administrators_root_entry();
  }

  template<IsAdministrationClient C>
  Beam::DirectoryEntry
      ToPythonAdministrationClient<C>::load_services_root_entry() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_services_root_entry();
  }

  template<IsAdministrationClient C>
  Beam::DirectoryEntry
      ToPythonAdministrationClient<C>::load_trading_groups_root_entry() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_trading_groups_root_entry();
  }

  template<IsAdministrationClient C>
  bool ToPythonAdministrationClient<C>::check_administrator(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->check_administrator(account);
  }

  template<IsAdministrationClient C>
  AccountRoles ToPythonAdministrationClient<C>::load_account_roles(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_roles(account);
  }

  template<IsAdministrationClient C>
  AccountRoles ToPythonAdministrationClient<C>::load_account_roles(
      const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_roles(parent, child);
  }

  template<IsAdministrationClient C>
  Beam::DirectoryEntry
      ToPythonAdministrationClient<C>::load_parent_trading_group(
        const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_parent_trading_group(account);
  }

  template<IsAdministrationClient C>
  AccountIdentity ToPythonAdministrationClient<C>::load_identity(
      const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_identity(account);
  }

  template<IsAdministrationClient C>
  void ToPythonAdministrationClient<C>::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    auto release = Beam::Python::GilRelease();
    m_client->store(account, identity);
  }

  template<IsAdministrationClient C>
  TradingGroup ToPythonAdministrationClient<C>::load_trading_group(
      const Beam::DirectoryEntry& directory) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_trading_group(directory);
  }

  template<IsAdministrationClient C>
  std::vector<Beam::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_managed_trading_groups(
        const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_managed_trading_groups(account);
  }

  template<IsAdministrationClient C>
  std::vector<Beam::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_administrators() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_administrators();
  }

  template<IsAdministrationClient C>
  std::vector<Beam::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_services() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_services();
  }

  template<IsAdministrationClient C>
  EntitlementDatabase ToPythonAdministrationClient<C>::load_entitlements() {
    auto release = Beam::Python::GilRelease();
    return m_client->load_entitlements();
  }

  template<IsAdministrationClient C>
  std::vector<Beam::DirectoryEntry>
      ToPythonAdministrationClient<C>::load_entitlements(
        const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_entitlements(account);
  }

  template<IsAdministrationClient C>
  void ToPythonAdministrationClient<C>::store_entitlements(
      const Beam::DirectoryEntry& account,
      const std::vector<Beam::DirectoryEntry>& entitlements) {
    auto release = Beam::Python::GilRelease();
    m_client->store_entitlements(account, entitlements);
  }

  template<IsAdministrationClient C>
  const Beam::Publisher<RiskParameters>&
      ToPythonAdministrationClient<C>::get_risk_parameters_publisher(
        const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->get_risk_parameters_publisher(account);
  }

  template<IsAdministrationClient C>
  void ToPythonAdministrationClient<C>::store(
      const Beam::DirectoryEntry& account, const RiskParameters& parameters) {
    auto release = Beam::Python::GilRelease();
    m_client->store(account, parameters);
  }

  template<IsAdministrationClient C>
  const Beam::Publisher<RiskState>&
      ToPythonAdministrationClient<C>::get_risk_state_publisher(
        const Beam::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->get_risk_state_publisher(account);
  }

  template<IsAdministrationClient C>
  void ToPythonAdministrationClient<C>::store(
      const Beam::DirectoryEntry& account, const RiskState& state) {
    auto release = Beam::Python::GilRelease();
    m_client->store(account, state);
  }

  template<IsAdministrationClient C>
  AccountModificationRequest
      ToPythonAdministrationClient<C>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_modification_request(id);
  }

  template<IsAdministrationClient C>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationClient<C>::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<IsAdministrationClient C>
  std::vector<AccountModificationRequest::Id> ToPythonAdministrationClient<C>::
      load_managed_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_managed_account_modification_request_ids(
      account, start_id, max_count);
  }

  template<IsAdministrationClient C>
  EntitlementModification
      ToPythonAdministrationClient<C>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_entitlement_modification(id);
  }

  template<IsAdministrationClient C>
  AccountModificationRequest ToPythonAdministrationClient<C>::submit(
      const Beam::DirectoryEntry& account,
      const EntitlementModification& modification, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->submit(account, modification, comment);
  }

  template<IsAdministrationClient C>
  RiskModification ToPythonAdministrationClient<C>::load_risk_modification(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_risk_modification(id);
  }

  template<IsAdministrationClient C>
  AccountModificationRequest ToPythonAdministrationClient<C>::submit(
      const Beam::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->submit(account, modification, comment);
  }

  template<IsAdministrationClient C>
  AccountModificationRequest::Update
      ToPythonAdministrationClient<C>::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_account_modification_request_status(id);
  }

  template<IsAdministrationClient C>
  AccountModificationRequest::Update
      ToPythonAdministrationClient<C>::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->approve_account_modification_request(id, comment);
  }

  template<IsAdministrationClient C>
  AccountModificationRequest::Update
      ToPythonAdministrationClient<C>::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->reject_account_modification_request(id, comment);
  }

  template<IsAdministrationClient C>
  Message ToPythonAdministrationClient<C>::load_message(Message::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_message(id);
  }

  template<IsAdministrationClient C>
  std::vector<Message::Id> ToPythonAdministrationClient<C>::load_message_ids(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->load_message_ids(id);
  }

  template<IsAdministrationClient C>
  Message ToPythonAdministrationClient<C>::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    auto release = Beam::Python::GilRelease();
    return m_client->send_account_modification_request_message(id, message);
  }

  template<IsAdministrationClient C>
  void ToPythonAdministrationClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
