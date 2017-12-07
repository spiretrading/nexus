#ifndef NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#define NEXUS_TO_PYTHON_ADMINISTRATION_CLIENT_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Python/GilRelease.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class ToPythonAdministrationClient
      \brief Wraps an AdministrationClient for use with Python.
      \tparam ClientType The type of AdministrationClient to wrap.
   */
  template<typename ClientType>
  class ToPythonAdministrationClient : public VirtualAdministrationClient {
    public:

      //! The type of AdministrationClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonAdministrationClient.
      /*!
        \param client The AdministrationClient to wrap.
      */
      ToPythonAdministrationClient(std::unique_ptr<Client> client);

      virtual ~ToPythonAdministrationClient() override final;

      virtual bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account) override final;

      virtual AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account) override final;

      virtual AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) override final;

      virtual Beam::ServiceLocator::DirectoryEntry LoadTradingGroupEntry(
        const Beam::ServiceLocator::DirectoryEntry& account) override final;

      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override final;

      virtual void StoreIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override final;

      virtual TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory) override final;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account) override final;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAdministrators() override final;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadServices() override final;

      virtual MarketDataService::EntitlementDatabase
        LoadEntitlements() override final;

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) override final;

      virtual void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        entitlements) override final;

      virtual const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override final;

      virtual void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override final;

      virtual const Beam::Publisher<RiskService::RiskState>&
        GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override final;

      virtual void StoreRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override final;

      virtual AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override final;

      virtual std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override final;

      virtual std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override final;

      virtual EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override final;

      virtual AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
        const EntitlementModification& modification,
        const Message& comment) override final;

      virtual AccountModificationRequest::Update
        LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override final;

      virtual AccountModificationRequest::Update
        ApproveAccountModificationRequest(AccountModificationRequest::Id id,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Message& comment) override final;

      virtual AccountModificationRequest::Update
        RejectAccountModificationRequest(AccountModificationRequest::Id id,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Message& comment) override final;

      virtual Message LoadMessage(Message::Id id) override final;

      virtual std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override final;

      virtual Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id,
        const Message& message) override final;

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  //! Makes a ToPythonAdministrationClient.
  /*!
    \param client The AdministrationClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonAdministrationClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonAdministrationClient<Client>>(
      std::move(client));
  }

  template<typename ClientType>
  ToPythonAdministrationClient<ClientType>::ToPythonAdministrationClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonAdministrationClient<ClientType>::~ToPythonAdministrationClient() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    Close();
    m_client.reset();
  }

  template<typename ClientType>
  bool ToPythonAdministrationClient<ClientType>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->CheckAdministrator(account);
  }

  template<typename ClientType>
  AccountRoles ToPythonAdministrationClient<ClientType>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadAccountRoles(account);
  }

  template<typename ClientType>
  AccountRoles ToPythonAdministrationClient<ClientType>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadAccountRoles(parent, child);
  }

  template<typename ClientType>
  Beam::ServiceLocator::DirectoryEntry ToPythonAdministrationClient<
      ClientType>::LoadTradingGroupEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadTradingGroupEntry(account);
  }

  template<typename ClientType>
  AccountIdentity ToPythonAdministrationClient<ClientType>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadIdentity(account);
  }

  template<typename ClientType>
  void ToPythonAdministrationClient<ClientType>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->StoreIdentity(account, identity);
  }

  template<typename ClientType>
  TradingGroup ToPythonAdministrationClient<ClientType>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadTradingGroup(directory);
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<ClientType>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadManagedTradingGroups(account);
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<ClientType>::LoadAdministrators() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadAdministrators();
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<ClientType>::LoadServices() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadServices();
  }

  template<typename ClientType>
  MarketDataService::EntitlementDatabase
      ToPythonAdministrationClient<ClientType>::LoadEntitlements() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadEntitlements();
  }

  template<typename ClientType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<ClientType>::LoadEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadEntitlements(account);
  }

  template<typename ClientType>
  void ToPythonAdministrationClient<ClientType>::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->StoreEntitlements(account, entitlements);
  }

  template<typename ClientType>
  const Beam::Publisher<RiskService::RiskParameters>&
      ToPythonAdministrationClient<ClientType>::GetRiskParametersPublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->GetRiskParametersPublisher(account);
  }

  template<typename ClientType>
  void ToPythonAdministrationClient<ClientType>::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->StoreRiskParameters(account, riskParameters);
  }

  template<typename ClientType>
  const Beam::Publisher<RiskService::RiskState>&
      ToPythonAdministrationClient<ClientType>::GetRiskStatePublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->GetRiskStatePublisher(account);
  }

  template<typename ClientType>
  void ToPythonAdministrationClient<ClientType>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    m_client->StoreRiskState(account, riskState);
  }

  template<typename ClientType>
  AccountModificationRequest ToPythonAdministrationClient<ClientType>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadAccountModificationRequest(id);
  }

  template<typename ClientType>
  std::vector<AccountModificationRequest::Id> ToPythonAdministrationClient<
      ClientType>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename ClientType>
  std::vector<AccountModificationRequest::Id> ToPythonAdministrationClient<
      ClientType>::LoadManagedAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadManagedAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename ClientType>
  EntitlementModification ToPythonAdministrationClient<ClientType>::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadEntitlementModification(id);
  }

  template<typename ClientType>
  AccountModificationRequest ToPythonAdministrationClient<ClientType>::
      SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
      const EntitlementModification& modification, const Message& comment) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->SubmitAccountModificationRequest(account,
      submissionAccount, modification, comment);
  }

  template<typename ClientType>
  AccountModificationRequest::Update ToPythonAdministrationClient<ClientType>::
      LoadAccountModificationRequestStatus(AccountModificationRequest::Id id) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadAccountModificationRequestStatus(id);
  }

  template<typename ClientType>
  AccountModificationRequest::Update ToPythonAdministrationClient<ClientType>::
      ApproveAccountModificationRequest(AccountModificationRequest::Id id,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Message& comment) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->ApproveAccountModificationRequest(id, account, comment);
  }

  template<typename ClientType>
  AccountModificationRequest::Update ToPythonAdministrationClient<ClientType>::
      RejectAccountModificationRequest(AccountModificationRequest::Id id,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Message& comment) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->RejectAccountModificationRequest(id, account, comment);
  }

  template<typename ClientType>
  Message ToPythonAdministrationClient<ClientType>::LoadMessage(
      Message::Id id) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadMessage(id);
  }

  template<typename ClientType>
  std::vector<Message::Id> ToPythonAdministrationClient<ClientType>::
      LoadMessageIds(AccountModificationRequest::Id id) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->LoadMessageIds(id);
  }

  template<typename ClientType>
  Message ToPythonAdministrationClient<ClientType>::
      SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
      const Message& message) {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    return m_client->SendAccountModificationRequestMessage(id, message);
  }

  template<typename ClientType>
  void ToPythonAdministrationClient<ClientType>::Open() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ClientType>
  void ToPythonAdministrationClient<ClientType>::Close() {
    Beam::Python::GilRelease gil;
    boost::lock_guard<Beam::Python::GilRelease> lock{gil};
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonAdministrationClient<ClientType>::Shutdown() {
    m_client->Close();
    m_openState.SetClosed();
  }
}
}

#endif
