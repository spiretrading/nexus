#ifndef NEXUS_VIRTUALADMINISTRATIONCLIENT_HPP
#define NEXUS_VIRTUALADMINISTRATIONCLIENT_HPP
#include <memory>
#include <vector>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queues.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class VirtualAdministrationClient
      \brief Provides a pure virtual interface to an AdministrationClient.
   */
  class VirtualAdministrationClient : private boost::noncopyable {
    public:
      virtual ~VirtualAdministrationClient();

      virtual bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

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

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualAdministrationClient.
      VirtualAdministrationClient();
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
      typedef typename Beam::TryDereferenceType<ClientType>::type Client;

      //! Constructs a WrapperAdministrationClient.
      /*!
        \param client The AdministrationClient to wrap.
      */
      template<typename AdministrationClientForward>
      WrapperAdministrationClient(AdministrationClientForward&& client);

      virtual ~WrapperAdministrationClient();

      virtual bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual Beam::ServiceLocator::DirectoryEntry LoadTradingGroupEntry(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual void StoreIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);

      virtual TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory);

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account);

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAdministrators();

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry> LoadServices();

      virtual MarketDataService::EntitlementDatabase LoadEntitlements();

      virtual std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements);

      virtual const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);

      virtual const Beam::Publisher<RiskService::RiskState>&
        GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      virtual void StoreRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState);

      virtual void Open();

      virtual void Close();

    private:
      typename Beam::OptionalLocalPtr<ClientType>::type m_client;
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

  inline VirtualAdministrationClient::~VirtualAdministrationClient() {}

  inline VirtualAdministrationClient::VirtualAdministrationClient() {}

  template<typename ClientType>
  template<typename AdministrationClientForward>
  WrapperAdministrationClient<ClientType>::WrapperAdministrationClient(
      AdministrationClientForward&& client)
      : m_client(std::forward<AdministrationClientForward>(client)) {}

  template<typename ClientType>
  WrapperAdministrationClient<ClientType>::~WrapperAdministrationClient() {}

  template<typename ClientType>
  bool WrapperAdministrationClient<ClientType>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return m_client->CheckAdministrator(account);
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
  void WrapperAdministrationClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperAdministrationClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
