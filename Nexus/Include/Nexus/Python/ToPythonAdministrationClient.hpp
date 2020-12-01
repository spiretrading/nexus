#ifndef NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#define NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/AdministrationService/AdministrationClientBox.hpp"

namespace Nexus::AdministrationService {

  /**
   * Wraps an AdministrationClient for use with Python.
   * @tparam <C> The type of AdministrationClient to wrap.
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
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadAccountsByRoles(
        AccountRoles roles);

      Beam::ServiceLocator::DirectoryEntry LoadAdministratorsRootEntry();

      Beam::ServiceLocator::DirectoryEntry LoadServicesRootEntry();

      Beam::ServiceLocator::DirectoryEntry LoadTradingGroupsRootEntry();

      bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account);

      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);

      Beam::ServiceLocator::DirectoryEntry LoadParentTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& account);

      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void StoreIdentity(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);

      TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory);

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
          account);

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadAdministrators();

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadServices();

      MarketDataService::EntitlementDatabase LoadEntitlements();

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
          entitlements);

      const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
          const Beam::ServiceLocator::DirectoryEntry& account);

      void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);

      const Beam::Publisher<RiskService::RiskState>& GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      void StoreRiskState(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState);

      AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id);

      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id startId, int maxCount);

      std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id startId, int maxCount);

      EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id);

      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);

      RiskModification LoadRiskModification(
        AccountModificationRequest::Id id);

      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment);

      AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id);

      AccountModificationRequest::Update ApproveAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment);

      AccountModificationRequest::Update RejectAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment);

      Message LoadMessage(Message::Id id);

      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id);

      Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id, const Message& message);

      void Close();

    private:
      boost::optional<Client> m_client;

      ToPythonAdministrationClient(
        const ToPythonAdministrationClient&) = delete;
      ToPythonAdministrationClient& operator =(
        const ToPythonAdministrationClient&) = delete;
  };

  template<typename Client>
  ToPythonAdministrationClient(Client&&) ->
    ToPythonAdministrationClient<std::decay_t<Client>>;

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
      ToPythonAdministrationClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonAdministrationClient<C>::Client&
      ToPythonAdministrationClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadAccountsByRoles(AccountRoles roles) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAccountsByRoles(roles);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadAdministratorsRootEntry() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAdministratorsRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadServicesRootEntry() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadServicesRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadTradingGroupsRootEntry() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadTradingGroupsRootEntry();
  }

  template<typename C>
  bool ToPythonAdministrationClient<C>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->CheckAdministrator(account);
  }

  template<typename C>
  AccountRoles ToPythonAdministrationClient<C>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAccountRoles(account);
  }

  template<typename C>
  AccountRoles ToPythonAdministrationClient<C>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAccountRoles(parent, child);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadParentTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadParentTradingGroup(account);
  }

  template<typename C>
  AccountIdentity ToPythonAdministrationClient<C>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadIdentity(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto release = Beam::Python::GilRelease();
    m_client->StoreIdentity(account, identity);
  }

  template<typename C>
  TradingGroup ToPythonAdministrationClient<C>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadTradingGroup(directory);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadManagedTradingGroups(account);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadAdministrators() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAdministrators();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadServices() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadServices();
  }

  template<typename C>
  MarketDataService::EntitlementDatabase
      ToPythonAdministrationClient<C>::LoadEntitlements() {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadEntitlements();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadEntitlements(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto release = Beam::Python::GilRelease();
    m_client->StoreEntitlements(account, entitlements);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskParameters>&
      ToPythonAdministrationClient<C>::GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->GetRiskParametersPublisher(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    auto release = Beam::Python::GilRelease();
    m_client->StoreRiskParameters(account, riskParameters);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskState>&
      ToPythonAdministrationClient<C>::GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = Beam::Python::GilRelease();
    return m_client->GetRiskStatePublisher(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto release = Beam::Python::GilRelease();
    m_client->StoreRiskState(account, riskState);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAccountModificationRequest(id);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationClient<C>::LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationClient<C>::LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadManagedAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  EntitlementModification ToPythonAdministrationClient<C>::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadEntitlementModification(id);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::
      SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename C>
  RiskModification ToPythonAdministrationClient<C>::
      LoadRiskModification(AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadRiskModification(id);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::
      SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename C>
  AccountModificationRequest::Update ToPythonAdministrationClient<C>::
      LoadAccountModificationRequestStatus(AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadAccountModificationRequestStatus(id);
  }

  template<typename C>
  AccountModificationRequest::Update ToPythonAdministrationClient<C>::
      ApproveAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->ApproveAccountModificationRequest(id, comment);
  }

  template<typename C>
  AccountModificationRequest::Update ToPythonAdministrationClient<C>::
      RejectAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) {
    auto release = Beam::Python::GilRelease();
    return m_client->RejectAccountModificationRequest(id, comment);
  }

  template<typename C>
  Message ToPythonAdministrationClient<C>::LoadMessage(Message::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadMessage(id);
  }

  template<typename C>
  std::vector<Message::Id> ToPythonAdministrationClient<C>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_client->LoadMessageIds(id);
  }

  template<typename C>
  Message ToPythonAdministrationClient<C>::
      SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
        const Message& message) {
    auto release = Beam::Python::GilRelease();
    return m_client->SendAccountModificationRequestMessage(id, message);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
