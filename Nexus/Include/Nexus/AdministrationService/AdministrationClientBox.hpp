#ifndef NEXUS_ADMINISTRATION_CLIENT_BOX_HPP
#define NEXUS_ADMINISTRATION_CLIENT_BOX_HPP
#include <memory>
#include <type_traits>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
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

  /** Provides a generic interface over an arbitrary AdministrationClient. */
  class AdministrationClientBox {
    public:

      /**
       * Constructs an AdministrationClientBox of a specified type using
       * emplacement.
       * @param <T> The type of administration client to emplace.
       * @param args The arguments to pass to the emplaced administration
       *        client.
       */
      template<typename T, typename... Args>
      explicit AdministrationClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a AdministrationClientBox by copying an existing
       * administration client.
       * @param client The client to copy.
       */
      template<typename AdministrationClient>
      explicit AdministrationClientBox(AdministrationClient client);

      explicit AdministrationClientBox(AdministrationClientBox* client);

      explicit AdministrationClientBox(
        const std::shared_ptr<AdministrationClientBox>& client);

      explicit AdministrationClientBox(
        const std::unique_ptr<AdministrationClientBox>& client);

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
      struct VirtualAdministrationClient {
        virtual ~VirtualAdministrationClient() = default;
        virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
          LoadAccountsByRoles(AccountRoles roles) = 0;
        virtual Beam::ServiceLocator::DirectoryEntry
          LoadAdministratorsRootEntry() = 0;
        virtual Beam::ServiceLocator::DirectoryEntry
          LoadServicesRootEntry() = 0;
        virtual Beam::ServiceLocator::DirectoryEntry
          LoadTradingGroupsRootEntry() = 0;
        virtual bool CheckAdministrator(
          const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual AccountRoles LoadAccountRoles(
          const Beam::ServiceLocator::DirectoryEntry& account) = 0;
        virtual AccountRoles LoadAccountRoles(
          const Beam::ServiceLocator::DirectoryEntry& parent,
          const Beam::ServiceLocator::DirectoryEntry& child) = 0;
        virtual Beam::ServiceLocator::DirectoryEntry LoadParentTradingGroup(
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
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedAdministrationClient final : VirtualAdministrationClient {
        using AdministrationClient = C;
        Beam::GetOptionalLocalPtr<AdministrationClient> m_client;

        template<typename... Args>
        WrappedAdministrationClient(Args&&... args);
        std::vector<Beam::ServiceLocator::DirectoryEntry> LoadAccountsByRoles(
          AccountRoles roles) override;
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
        Beam::ServiceLocator::DirectoryEntry LoadParentTradingGroup(
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
        std::vector<Beam::ServiceLocator::DirectoryEntry>
          LoadServices() override;
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
          const RiskModification& modification,
          const Message& comment) override;
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
        void Close() override;
      };
      std::shared_ptr<VirtualAdministrationClient> m_client;
  };

  template<typename T, typename... Args>
  AdministrationClientBox::AdministrationClientBox(
    std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedAdministrationClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename AdministrationClient>
  AdministrationClientBox::AdministrationClientBox(AdministrationClient client)
    : AdministrationClientBox(std::in_place_type<AdministrationClient>,
        std::move(client)) {}

  inline AdministrationClientBox::AdministrationClientBox(
    AdministrationClientBox* client)
    : AdministrationClientBox(*client) {}

  inline AdministrationClientBox::AdministrationClientBox(
    const std::shared_ptr<AdministrationClientBox>& client)
    : AdministrationClientBox(*client) {}

  inline AdministrationClientBox::AdministrationClientBox(
    const std::unique_ptr<AdministrationClientBox>& client)
    : AdministrationClientBox(*client) {}

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::LoadAccountsByRoles(AccountRoles roles) {
    return m_client->LoadAccountsByRoles(roles);
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::LoadAdministratorsRootEntry() {
    return m_client->LoadAdministratorsRootEntry();
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::LoadServicesRootEntry() {
    return m_client->LoadServicesRootEntry();
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::LoadTradingGroupsRootEntry() {
    return m_client->LoadTradingGroupsRootEntry();
  }

  inline bool AdministrationClientBox::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->CheckAdministrator(account);
  }

  inline AccountRoles AdministrationClientBox::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadAccountRoles(account);
  }

  inline AccountRoles AdministrationClientBox::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return m_client->LoadAccountRoles(parent, child);
  }

  inline Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::LoadParentTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadParentTradingGroup(account);
  }

  inline AccountIdentity AdministrationClientBox::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadIdentity(account);
  }

  inline void AdministrationClientBox::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_client->StoreIdentity(account, identity);
  }

  inline TradingGroup AdministrationClientBox::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    return m_client->LoadTradingGroup(directory);
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadManagedTradingGroups(account);
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::LoadAdministrators() {
    return m_client->LoadAdministrators();
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::LoadServices() {
    return m_client->LoadServices();
  }

  inline MarketDataService::EntitlementDatabase
      AdministrationClientBox::LoadEntitlements() {
    return m_client->LoadEntitlements();
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadEntitlements(account);
  }

  inline void AdministrationClientBox::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    m_client->StoreEntitlements(account, entitlements);
  }

  inline const Beam::Publisher<RiskService::RiskParameters>&
      AdministrationClientBox::GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskParametersPublisher(account);
  }

  inline void AdministrationClientBox::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_client->StoreRiskParameters(account, riskParameters);
  }

  inline const Beam::Publisher<RiskService::RiskState>&
      AdministrationClientBox::GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskStatePublisher(account);
  }

  inline void AdministrationClientBox::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_client->StoreRiskState(account, riskState);
  }

  inline AccountModificationRequest AdministrationClientBox::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequest(id);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationClientBox::LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  inline std::vector<AccountModificationRequest::Id>
      AdministrationClientBox::LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadManagedAccountModificationRequestIds(account, startId,
      maxCount);
  }

  inline EntitlementModification AdministrationClientBox::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    return m_client->LoadEntitlementModification(id);
  }

  inline AccountModificationRequest AdministrationClientBox::
      SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  inline RiskModification AdministrationClientBox::LoadRiskModification(
      AccountModificationRequest::Id id) {
    return m_client->LoadRiskModification(id);
  }

  inline AccountModificationRequest AdministrationClientBox::
    SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  inline AccountModificationRequest::Update AdministrationClientBox::
      LoadAccountModificationRequestStatus(AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequestStatus(id);
  }

  inline AccountModificationRequest::Update AdministrationClientBox::
      ApproveAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) {
    return m_client->ApproveAccountModificationRequest(id, comment);
  }

  inline AccountModificationRequest::Update AdministrationClientBox::
      RejectAccountModificationRequest(AccountModificationRequest::Id id,
        const Message& comment) {
    return m_client->RejectAccountModificationRequest(id, comment);
  }

  inline Message AdministrationClientBox::LoadMessage(Message::Id id) {
    return m_client->LoadMessage(id);
  }

  inline std::vector<Message::Id> AdministrationClientBox::LoadMessageIds(
      AccountModificationRequest::Id id) {
    return m_client->LoadMessageIds(id);
  }

  inline Message AdministrationClientBox::SendAccountModificationRequestMessage(
      AccountModificationRequest::Id id, const Message& message) {
    return m_client->SendAccountModificationRequestMessage(id, message);
  }

  inline void AdministrationClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  AdministrationClientBox::WrappedAdministrationClient<C>::
    WrappedAdministrationClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadAccountsByRoles(AccountRoles roles) {
    return m_client->LoadAccountsByRoles(roles);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadAdministratorsRootEntry() {
    return m_client->LoadAdministratorsRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadServicesRootEntry() {
    return m_client->LoadServicesRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadTradingGroupsRootEntry() {
    return m_client->LoadTradingGroupsRootEntry();
  }

  template<typename C>
  bool AdministrationClientBox::WrappedAdministrationClient<C>::
      CheckAdministrator(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->CheckAdministrator(account);
  }

  template<typename C>
  AccountRoles AdministrationClientBox::WrappedAdministrationClient<C>::
      LoadAccountRoles(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadAccountRoles(account);
  }

  template<typename C>
  AccountRoles AdministrationClientBox::WrappedAdministrationClient<C>::
      LoadAccountRoles(const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) {
    return m_client->LoadAccountRoles(parent, child);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadParentTradingGroup(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadParentTradingGroup(account);
  }

  template<typename C>
  AccountIdentity AdministrationClientBox::WrappedAdministrationClient<C>::
      LoadIdentity(const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadIdentity(account);
  }

  template<typename C>
  void AdministrationClientBox::WrappedAdministrationClient<C>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    m_client->StoreIdentity(account, identity);
  }

  template<typename C>
  TradingGroup AdministrationClientBox::WrappedAdministrationClient<C>::
      LoadTradingGroup(const Beam::ServiceLocator::DirectoryEntry& directory) {
    return m_client->LoadTradingGroup(directory);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadManagedTradingGroups(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadManagedTradingGroups(account);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadAdministrators() {
    return m_client->LoadAdministrators();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::WrappedAdministrationClient<C>::LoadServices() {
    return m_client->LoadServices();
  }

  template<typename C>
  MarketDataService::EntitlementDatabase
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadEntitlements() {
    return m_client->LoadEntitlements();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClientBox::WrappedAdministrationClient<C>::LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->LoadEntitlements(account);
  }

  template<typename C>
  void AdministrationClientBox::WrappedAdministrationClient<C>::
      StoreEntitlements(const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    m_client->StoreEntitlements(account, entitlements);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskParameters>&
      AdministrationClientBox::WrappedAdministrationClient<C>::
        GetRiskParametersPublisher(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskParametersPublisher(account);
  }

  template<typename C>
  void AdministrationClientBox::WrappedAdministrationClient<C>::
      StoreRiskParameters(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) {
    m_client->StoreRiskParameters(account, riskParameters);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskState>&
      AdministrationClientBox::WrappedAdministrationClient<C>::
        GetRiskStatePublisher(
          const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->GetRiskStatePublisher(account);
  }

  template<typename C>
  void AdministrationClientBox::WrappedAdministrationClient<C>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    m_client->StoreRiskState(account, riskState);
  }

  template<typename C>
  AccountModificationRequest AdministrationClientBox::
      WrappedAdministrationClient<C>::LoadAccountModificationRequest(
        AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequest(id);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadAccountModificationRequestIds(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      AdministrationClientBox::WrappedAdministrationClient<C>::
        LoadManagedAccountModificationRequestIds(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id startId, int maxCount) {
    return m_client->LoadManagedAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  EntitlementModification AdministrationClientBox::
      WrappedAdministrationClient<C>::LoadEntitlementModification(
        AccountModificationRequest::Id id) {
    return m_client->LoadEntitlementModification(id);
  }

  template<typename C>
  AccountModificationRequest AdministrationClientBox::
      WrappedAdministrationClient<C>::SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename C>
  RiskModification AdministrationClientBox::WrappedAdministrationClient<C>::
      LoadRiskModification(AccountModificationRequest::Id id) {
    return m_client->LoadRiskModification(id);
  }

  template<typename C>
  AccountModificationRequest AdministrationClientBox::
      WrappedAdministrationClient<C>::SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    return m_client->SubmitAccountModificationRequest(account, modification,
      comment);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClientBox::
      WrappedAdministrationClient<C>::LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) {
    return m_client->LoadAccountModificationRequestStatus(id);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClientBox::
      WrappedAdministrationClient<C>::ApproveAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->ApproveAccountModificationRequest(id, comment);
  }

  template<typename C>
  AccountModificationRequest::Update AdministrationClientBox::
      WrappedAdministrationClient<C>::RejectAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_client->RejectAccountModificationRequest(id, comment);
  }

  template<typename C>
  Message AdministrationClientBox::WrappedAdministrationClient<C>::LoadMessage(
      Message::Id id) {
    return m_client->LoadMessage(id);
  }

  template<typename C>
  std::vector<Message::Id> AdministrationClientBox::
      WrappedAdministrationClient<C>::LoadMessageIds(
        AccountModificationRequest::Id id) {
    return m_client->LoadMessageIds(id);
  }

  template<typename C>
  Message AdministrationClientBox::WrappedAdministrationClient<C>::
      SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
        const Message& message) {
    return m_client->SendAccountModificationRequestMessage(id, message);
  }

  template<typename C>
  void AdministrationClientBox::WrappedAdministrationClient<C>::Close() {
    m_client->Close();
  }
}

#endif
