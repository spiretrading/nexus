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

  /*! \class VirtualAdministrationClient
      \brief Provides a pure virtual interface to an AdministrationClient.
   */
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

      //! Constructs a VirtualAdministrationClient.
      VirtualAdministrationClient() = default;
  };

  /*! \class WrapperAdministrationClient
      \brief Wraps an AdministrationClient providing it with a virtual
             interface.
      \tparam ClientType The type of AdministrationClient to wrap.
   */
  template<typename ClientType>
  class WrapperAdministrationClient : public VirtualAdministrationClient {
    public:

      //! The AdministrationClient to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperAdministrationClient.
      /*!
        \param client The AdministrationClient to wrap.
      */
      template<typename AdministrationClientForward>
      WrapperAdministrationClient(AdministrationClientForward&& client);

      virtual ~WrapperAdministrationClient();

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAccountsByRoles(AccountRoles roles) override;

      virtual Beam::ServiceLocator::DirectoryEntry
        LoadAdministratorsRootEntry() override;

      virtual Beam::ServiceLocator::DirectoryEntry
        LoadServicesRootEntry() override;

      virtual Beam::ServiceLocator::DirectoryEntry
        LoadTradingGroupsRootEntry() override;

      virtual bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) override;

      virtual Beam::ServiceLocator::DirectoryEntry LoadTradingGroupEntry(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void StoreIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      virtual TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory) override;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account) override;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAdministrators() override;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadServices() override;

      virtual MarketDataService::EntitlementDatabase
        LoadEntitlements() override;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        entitlements) override;

      virtual const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      virtual const Beam::Publisher<RiskService::RiskState>&
        GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      virtual void StoreRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override;

      virtual AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override;

      virtual std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      virtual std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      virtual EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override;

      virtual AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification,
        const Message& comment) override;

      virtual RiskModification LoadRiskModification(
        AccountModificationRequest::Id id) override;

      virtual AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) override;

      virtual AccountModificationRequest::Update
        LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override;

      virtual AccountModificationRequest::Update
        ApproveAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) override;

      virtual AccountModificationRequest::Update
        RejectAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) override;

      virtual Message LoadMessage(Message::Id id) override;

      virtual std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override;

      virtual Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id, const Message& message) override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      Beam::GetOptionalLocalPtr<ClientType> m_client;
  };

  //! Wraps an AdministrationClient into a VirtualAdministrationClient.
  /*!
    \param client The client to wrap.
  */
  template<typename AdministrationClient>
  std::unique_ptr<VirtualAdministrationClient> MakeVirtualAdministrationClient(
      AdministrationClient&& client) {
    return std::make_unique<WrapperAdministrationClient<
      AdministrationClient>>(std::forward<AdministrationClient>(client));
  }

  template<typename ClientType>
  template<typename AdministrationClientForward>
  WrapperAdministrationClient<ClientType>::WrapperAdministrationClient(
      AdministrationClientForward&& client)
      : m_client{std::forward<AdministrationClientForward>(client)} {}

  template<typename ClientType>
  WrapperAdministrationClient<ClientType>::~WrapperAdministrationClient() {
    Close();
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<ClientType>::LoadAccountsByRoles(
      AccountRoles roles) {
    return m_client->LoadAccountsByRoles(roles);
  }

  template<typename ClientType>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<ClientType>::
      LoadAdministratorsRootEntry() {
    return m_client->LoadAdministratorsRootEntry();
  }

  template<typename ClientType>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<ClientType>::
      LoadServicesRootEntry() {
    return m_client->LoadServicesRootEntry();
  }

  template<typename ClientType>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<ClientType>::
      LoadTradingGroupsRootEntry() {
    return m_client->LoadTradingGroupsRootEntry();
  }

  template<typename ClientType>
  bool WrapperAdministrationClient<ClientType>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->CheckAdministrator(account);
  }

  template<typename ClientType>
  AccountRoles WrapperAdministrationClient<ClientType>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadAccountRoles(account);
  }

  template<typename ClientType>
  AccountRoles WrapperAdministrationClient<ClientType>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return m_client->LoadAccountRoles(parent, child);
  }

  template<typename ClientType>
  Beam::ServiceLocator::DirectoryEntry WrapperAdministrationClient<ClientType>::
      LoadTradingGroupEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadTradingGroupEntry(account);
  }

  template<typename ClientType>
  AccountIdentity WrapperAdministrationClient<ClientType>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadIdentity(account);
  }

  template<typename ClientType>
  void WrapperAdministrationClient<ClientType>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_client->StoreIdentity(account, identity);
  }

  template<typename ClientType>
  TradingGroup WrapperAdministrationClient<ClientType>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    return m_client->LoadTradingGroup(directory);
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<ClientType>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadManagedTradingGroups(account);
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<ClientType>::LoadAdministrators() {
    return m_client->LoadAdministrators();
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<ClientType>::LoadServices() {
    return m_client->LoadServices();
  }

  template<typename ClientType>
  MarketDataService::EntitlementDatabase
      WrapperAdministrationClient<ClientType>::LoadEntitlements() {
    return m_client->LoadEntitlements();
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      WrapperAdministrationClient<ClientType>::LoadEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadEntitlements(account);
  }

  template<typename ClientType>
  void WrapperAdministrationClient<ClientType>::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    m_client->StoreEntitlements(account, entitlements);
  }

  template<typename ClientType>
  const Beam::Publisher<RiskService::RiskParameters>&
      WrapperAdministrationClient<ClientType>::GetRiskParametersPublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskParametersPublisher(account);
  }

  template<typename ClientType>
  void WrapperAdministrationClient<ClientType>::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_client->StoreRiskParameters(account, riskParameters);
  }

  template<typename ClientType>
  const Beam::Publisher<RiskService::RiskState>&
      WrapperAdministrationClient<ClientType>::GetRiskStatePublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskStatePublisher(account);
  }

  template<typename ClientType>
  void WrapperAdministrationClient<ClientType>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_client->StoreRiskState(account, riskState);
  }

  template<typename ClientType>
  AccountModificationRequest WrapperAdministrationClient<ClientType>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequest(id);
  }

  template<typename ClientType>
  std::vector<AccountModificationRequest::Id> WrapperAdministrationClient<
      ClientType>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename ClientType>
  std::vector<AccountModificationRequest::Id> WrapperAdministrationClient<
      ClientType>::LoadManagedAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadManagedAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename ClientType>
  EntitlementModification WrapperAdministrationClient<ClientType>::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    return m_client->LoadEntitlementModification(id);
  }

  template<typename ClientType>
  AccountModificationRequest WrapperAdministrationClient<ClientType>::
      SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const EntitlementModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename ClientType>
  RiskModification WrapperAdministrationClient<ClientType>::
      LoadRiskModification(AccountModificationRequest::Id id) {
    return m_client->LoadRiskModification(id);
  }

  template<typename ClientType>
  AccountModificationRequest WrapperAdministrationClient<ClientType>::
      SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename ClientType>
  AccountModificationRequest::Update WrapperAdministrationClient<ClientType>::
      LoadAccountModificationRequestStatus(AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequestStatus(id);
  }

  template<typename ClientType>
  AccountModificationRequest::Update WrapperAdministrationClient<ClientType>::
      ApproveAccountModificationRequest(AccountModificationRequest::Id id,
      const Message& comment) {
    return m_client->ApproveAccountModificationRequest(id, comment);
  }

  template<typename ClientType>
  AccountModificationRequest::Update WrapperAdministrationClient<ClientType>::
      RejectAccountModificationRequest(AccountModificationRequest::Id id,
      const Message& comment) {
    return m_client->RejectAccountModificationRequest(id, comment);
  }

  template<typename ClientType>
  Message WrapperAdministrationClient<ClientType>::LoadMessage(Message::Id id) {
    return m_client->LoadMessage(id);
  }

  template<typename ClientType>
  std::vector<Message::Id> WrapperAdministrationClient<ClientType>::
      LoadMessageIds(AccountModificationRequest::Id id) {
    return m_client->LoadMessageIds(id);
  }

  template<typename ClientType>
  Message WrapperAdministrationClient<ClientType>::
      SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
      const Message& message) {
    return m_client->SendAccountModificationRequestMessage(id, message);
  }

  template<typename ClientType>
  void WrapperAdministrationClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperAdministrationClient<ClientType>::Close() {
    m_client->Close();
  }
}

#endif
