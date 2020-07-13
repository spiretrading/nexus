#ifndef NEXUS_VIRTUAL_ADMINISTRATION_CLIENT_HPP
#define NEXUS_VIRTUAL_ADMINISTRATION_CLIENT_HPP
#include <memory>
#include <vector>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus::AdministrationService {

  /** Provides a pure virtual interface to an AdministrationClient. */
  class VirtualAdministrationClient : private boost::noncopyable {
    public:
      virtual ~VirtualAdministrationClient() = default;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAccountsByRoles(AccountRoles roles) = 0;

      virtual Beam::ServiceLocator::DirectoryEntry
        LoadAdministratorsRootEntry() = 0;

      virtual Beam::ServiceLocator::DirectoryEntry LoadServicesRootEntry() = 0;

      virtual Beam::ServiceLocator::DirectoryEntry
        LoadTradingGroupsRootEntry() = 0;

      virtual bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) = 0;

      virtual Beam::ServiceLocator::DirectoryEntry LoadTradingGroupEntry(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual void StoreIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) = 0;

      virtual TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory) = 0;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account) = 0;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAdministrators() = 0;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadServices() = 0;

      virtual MarketDataService::EntitlementDatabase LoadEntitlements() = 0;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        entitlements) = 0;

      virtual const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) = 0;

      virtual const Beam::Publisher<RiskService::RiskState>&
        GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      virtual void StoreRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) = 0;

      virtual AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) = 0;

      virtual std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) = 0;

      virtual std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) = 0;

      virtual EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) = 0;

      virtual AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification,
        const Message& comment) = 0;

      virtual RiskModification LoadRiskModification(
        AccountModificationRequest::Id id) = 0;

      virtual AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) = 0;

      virtual AccountModificationRequest::Update
        LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) = 0;

      virtual AccountModificationRequest::Update
        ApproveAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) = 0;

      virtual AccountModificationRequest::Update
        RejectAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) = 0;

      virtual Message LoadMessage(Message::Id id) = 0;

      virtual std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) = 0;

      virtual Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id, const Message& message) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualAdministrationClient. */
      VirtualAdministrationClient() = default;
  };

  /**
   * Wraps an AdministrationClient providing it with a virtual interface.
   * @param <C> The type of AdministrationClient to wrap.
   */
  template<typename C>
  class WrapperAdministrationClient : public VirtualAdministrationClient {
    public:

      /** The AdministrationClient to wrap. */
      using Client = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperAdministrationClient.
       * @param client The AdministrationClient to wrap.
       */
      template<typename CF>
      WrapperAdministrationClient(CF&& client);

      ~WrapperAdministrationClient() override;

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAccountsByRoles(AccountRoles roles) override;

      Beam::ServiceLocator::DirectoryEntry
        LoadAdministratorsRootEntry() override;

      Beam::ServiceLocator::DirectoryEntry LoadServicesRootEntry() override;

      Beam::ServiceLocator::DirectoryEntry
        LoadTradingGroupsRootEntry() override;

      bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) override;

      Beam::ServiceLocator::DirectoryEntry LoadTradingGroupEntry(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreIdentity(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory) override;

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account) override;

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAdministrators() override;

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadServices() override;

      MarketDataService::EntitlementDatabase LoadEntitlements() override;

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        entitlements) override;

      const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      const Beam::Publisher<RiskService::RiskState>& GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreRiskState(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override;

      AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override;

      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override;

      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification,
        const Message& comment) override;

      RiskModification LoadRiskModification(
        AccountModificationRequest::Id id) override;

      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) override;

      AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override;

      AccountModificationRequest::Update ApproveAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment) override;

      AccountModificationRequest::Update RejectAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment) override;

      Message LoadMessage(Message::Id id) override;

      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override;

      Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id, const Message& message) override;

      void Open() override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<C> m_client;
  };

  /**
   * Wraps an AdministrationClient into a VirtualAdministrationClient.
   * @param client The client to wrap.
   */
  template<typename AdministrationClient>
  std::unique_ptr<VirtualAdministrationClient> MakeVirtualAdministrationClient(
      AdministrationClient&& client) {
    return std::make_unique<WrapperAdministrationClient<
      AdministrationClient>>(std::forward<AdministrationClient>(client));
  }

  template<typename C>
  template<typename CF>
  WrapperAdministrationClient<C>::WrapperAdministrationClient(CF&& client)
    : m_client(std::forward<CF>(client)) {}

  template<typename C>
  WrapperAdministrationClient<C>::~WrapperAdministrationClient() {
    Close();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<C>::LoadAccountsByRoles(AccountRoles roles) {
    return m_client->LoadAccountsByRoles(roles);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<C>::
      LoadAdministratorsRootEntry() {
    return m_client->LoadAdministratorsRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<C>::
      LoadServicesRootEntry() {
    return m_client->LoadServicesRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<C>::
      LoadTradingGroupsRootEntry() {
    return m_client->LoadTradingGroupsRootEntry();
  }

  template<typename C>
  bool WrapperAdministrationClient<C>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->CheckAdministrator(account);
  }

  template<typename C>
  AccountRoles WrapperAdministrationClient<C>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadAccountRoles(account);
  }

  template<typename C>
  AccountRoles WrapperAdministrationClient<C>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return m_client->LoadAccountRoles(parent, child);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<C>::
      LoadTradingGroupEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadTradingGroupEntry(account);
  }

  template<typename C>
  AccountIdentity WrapperAdministrationClient<C>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadIdentity(account);
  }

  template<typename C>
  void WrapperAdministrationClient<C>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_client->StoreIdentity(account, identity);
  }

  template<typename C>
  TradingGroup WrapperAdministrationClient<C>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    return m_client->LoadTradingGroup(directory);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<C>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadManagedTradingGroups(account);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<C>::LoadAdministrators() {
    return m_client->LoadAdministrators();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<C>::LoadServices() {
    return m_client->LoadServices();
  }

  template<typename C>
  MarketDataService::EntitlementDatabase
      WrapperAdministrationClient<C>::LoadEntitlements() {
    return m_client->LoadEntitlements();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<C>::LoadEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadEntitlements(account);
  }

  template<typename C>
  void WrapperAdministrationClient<C>::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    m_client->StoreEntitlements(account, entitlements);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskParameters>&
      WrapperAdministrationClient<C>::GetRiskParametersPublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskParametersPublisher(account);
  }

  template<typename C>
  void WrapperAdministrationClient<C>::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_client->StoreRiskParameters(account, riskParameters);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskState>&
      WrapperAdministrationClient<C>::GetRiskStatePublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskStatePublisher(account);
  }

  template<typename C>
  void WrapperAdministrationClient<C>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_client->StoreRiskState(account, riskState);
  }

  template<typename C>
  AccountModificationRequest WrapperAdministrationClient<C>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequest(id);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      WrapperAdministrationClient<C>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      WrapperAdministrationClient<C>::LoadManagedAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadManagedAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  EntitlementModification WrapperAdministrationClient<C>::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    return m_client->LoadEntitlementModification(id);
  }

  template<typename C>
  AccountModificationRequest WrapperAdministrationClient<C>::
      SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const EntitlementModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename C>
  RiskModification WrapperAdministrationClient<C>::LoadRiskModification(
      AccountModificationRequest::Id id) {
    return m_client->LoadRiskModification(id);
  }

  template<typename C>
  AccountModificationRequest WrapperAdministrationClient<C>::
      SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename C>
  AccountModificationRequest::Update WrapperAdministrationClient<C>::
      LoadAccountModificationRequestStatus(AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequestStatus(id);
  }

  template<typename C>
  AccountModificationRequest::Update WrapperAdministrationClient<C>::
      ApproveAccountModificationRequest(AccountModificationRequest::Id id,
      const Message& comment) {
    return m_client->ApproveAccountModificationRequest(id, comment);
  }

  template<typename C>
  AccountModificationRequest::Update WrapperAdministrationClient<C>::
      RejectAccountModificationRequest(AccountModificationRequest::Id id,
      const Message& comment) {
    return m_client->RejectAccountModificationRequest(id, comment);
  }

  template<typename C>
  Message WrapperAdministrationClient<C>::LoadMessage(Message::Id id) {
    return m_client->LoadMessage(id);
  }

  template<typename C>
  std::vector<Message::Id> WrapperAdministrationClient<C>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    return m_client->LoadMessageIds(id);
  }

  template<typename C>
  Message WrapperAdministrationClient<C>::
      SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
      const Message& message) {
    return m_client->SendAccountModificationRequestMessage(id, message);
  }

  template<typename C>
  void WrapperAdministrationClient<C>::Open() {
    m_client->Open();
  }

  template<typename C>
  void WrapperAdministrationClient<C>::Close() {
    m_client->Close();
  }
}

#endif
