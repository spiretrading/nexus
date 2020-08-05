#ifndef NEXUS_ADMINISTRATION_CLIENT_HPP
#define NEXUS_ADMINISTRATION_CLIENT_HPP
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/AdministrationServices.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/RiskModification.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus::AdministrationService {

  /** Used to push updates about an account's RiskState. */
  using RiskStateQueue = Beam::AbstractQueue<RiskService::RiskState>;

  /** Used to push updates about an account's RiskParameters. */
  using RiskParametersQueue = Beam::AbstractQueue<RiskService::RiskParameters>;

  /**
   * Client used to access the Nexus Administration service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class AdministrationClient : private boost::noncopyable {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs an AdministrationClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      AdministrationClient(BF&& clientBuilder);

      ~AdministrationClient();

      /**
       * Loads the list of accounts that match a set of roles.
       * @param roles The roles to match.
       * @return The list of directory entries of accounts that match the
       *         specified roles.
       */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadAccountsByRoles(
        AccountRoles roles);

      /** Loads the DirectoryEntry containing all administrators. */
      Beam::ServiceLocator::DirectoryEntry LoadAdministratorsRootEntry();

      /** Loads the DirectoryEntry containing all service accounts. */
      Beam::ServiceLocator::DirectoryEntry LoadServicesRootEntry();

      /** Loads the DirectoryEntry containing all trading groups. */
      Beam::ServiceLocator::DirectoryEntry LoadTradingGroupsRootEntry();

      /**
       * Returns <code>true</code> iff an account is an administrator.
       * @param account The account to test.
       * @return <code>true</code> iff the <i>account</i> is an administrator.
       */
      bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Returns an accounts roles.
       * @param account The account to lookup.
       * @return The roles associated with the <i>account</i>.
       */
      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Returns the roles one account has over another.
       * @param parent The account whose roles are to be loaded.
       * @param child The account being supervised.
       * @return The roles that the <i>parent</i> account has over the
       *         <i>child</i> account.
       */
      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);

      /**
       * Loads the DirectoryEntry representing an account's trading group.
       * @param account The account whose trading group is to be loaded.
       * @return The directory of the <i>account</i>'s trading group.
       */
      Beam::ServiceLocator::DirectoryEntry LoadParentTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Loads an account's identity.
       * @param account The account whose identity is to be loaded.
       * @return The AccountIdentity for the specified <i>account</i>.
       */
      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Sets the identity for an account.
       * @param account The account to set the identity for.
       * @param identity The AccountIdentity to assign to the <i>account</i>.
       */
      void StoreIdentity(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);

      /**
       * Loads a TradingGroup from its DirectoryEntry.
       * @param directory The DirectoryEntry of the TradingGroup to load.
       * @return The TradingGroup represented by the specified <i>directory</i>.
       */
      TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory);

      /**
       * Loads the DirectoryEntries of TradingGroups managed by an account.
       * @param account The account to load the TradingGroups from.
       * @return The list of TradingGroups managed by the <i>account</i>.
       */
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account);

      /**
       * Loads the system administrators.
       * @return The list of system administrators.
       */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadAdministrators();

      /**
       * Loads the accounts providing system services.
       * @return The list of accounts providing system services.
       */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadServices();

      /**
       * Loads the EntitlementDatabase.
       * @return The EntitlementDatabase.
       */
      MarketDataService::EntitlementDatabase LoadEntitlements();

      /**
       * Loads the entitlements granted to an account.
       * @param account The account to load the entitlements for.
       * @return The list of entitlements granted to the <i>account</i>.
       */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Sets an account's entitlements.
       * @param account The account of the entitlements to set.
       * @param entitlements The list of entitlements to grant to the
       *        <i>account</i>.
       */
      void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements);

      /**
       * Returns the object publishing an account's RiskParameters.
       * @param account The account to monitor.
       */
      const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Sets an account's RiskParameters.
       * @param account The account whose RiskParameters are to be set.
       * @param riskParameters The RiskParameters to assign to the
       *        <i>account</i>.
       */
      void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);

      /**
       * Returns the object publishing an account's RiskState.
       * @param account The account to monitor.
       */
      const Beam::Publisher<RiskService::RiskState>& GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Sets an account's RiskState.
       * @param account The account to set RiskState of.
       * @param riskState The <i>account</i>'s current RiskState.
       */
      void StoreRiskState(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState);

      /**
       * Loads an account modification request.
       * @param id The id of the request to load.
       * @return The request with the specified <i>id</i>.
       */
      AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id);

      /**
       * Given an account, loads the ids of requests to modify that account.
       * @param account The account whose requests are to be loaded.
       * @param startId The id of the first request to load (exclusive) or -1
       *        to start with the most recent request.
       * @param maxCount The maximum number of ids to load.
       * @return The list of account modification requests.
       */
      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount);

      /**
       * Given an account, loads the ids of requests that the account is
       * authorized to manage.
       * @param account The account managing modifications.
       * @param startId The id of the first request to load (exclusive) or -1
       *        to start with the most recent request.
       * @param maxCount The maximum number of ids to load.
       * @return The list of account modification requests.
       */
      std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount);

      /**
       * Loads an entitlement modification.
       * @param id The id of the request to load.
       * @return The entitlement modification with the specified <i>id</i>.
       */
      EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id);

      /**
       * Submits a request to modify an account's entitlements.
       * @param account The account to modify.
       * @param modification The modification to apply.
       * @param comment The comment to associate with the request.
       * @return An object representing the request.
       */
      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);

      /**
       * Loads a risk modification.
       * @param id The id of the request to load.
       * @return The risk modification with the specified <i>id</i>.
       */
      RiskModification LoadRiskModification(AccountModificationRequest::Id id);

      /**
       * Submits a request to modify an account's risk parameters.
       * @param account The account to modify.
       * @param modification The modification to apply.
       * @param comment The comment to associate with the request.
       * @return An object representing the request.
       */
      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment);

      /**
       * Loads the status of an account modification request.
       * @param id The id of the request.
       * @return The update representing the current status of the request.
       */
      AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id);

      /**
       * Approves an account modification request.
       * @param id The id of the request to approve.
       * @param comment The comment to associate with the update.
       * @return An object representing the update.
       */
      AccountModificationRequest::Update ApproveAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment);

      /**
       * Rejects an account modification request.
       * @param id The id of the request to reject.
       * @param comment The comment to associate with the update.
       * @return An object representing the update.
       */
      AccountModificationRequest::Update RejectAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment);

      /**
       * Loads a message.
       * @param id The id of the message.
       * @return The message with the specified <i>id</i>.
       */
      Message LoadMessage(Message::Id id);

      /**
       * Loads the list of messages associated with an account modification.
       * @param id The id of the request.
       * @return A list of message ids associated with the request.
       */
      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id);

      /**
       * Appends a message to an account modification request.
       * @param id The id of the request to send the message to.
       * @param message The message to append.
       * @return The appended message.
       */
      Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id, const Message& message);

      void Open();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using RiskParameterPublisher = Beam::StatePublisher<
        RiskService::RiskParameters>;
      using RiskStatePublisher = Beam::StatePublisher<RiskService::RiskState>;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::IO::OpenState m_openState;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskParameterPublisher>> m_riskParameterPublishers;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskStatePublisher>> m_riskStatePublishers;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void RecoverRiskParameters(ServiceProtocolClient& client);
      void RecoverRiskState(ServiceProtocolClient& client);
      void OnRiskParametersMessage(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);
      void OnRiskStateMessage(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        RiskService::RiskState riskState);
  };

  template<typename B>
  template<typename BF>
  AdministrationClient<B>::AdministrationClient(BF&& clientBuilder)
      : m_clientHandler(std::forward<BF>(clientBuilder)) {
    m_clientHandler.SetReconnectHandler(
      std::bind(&AdministrationClient::OnReconnect, this,
      std::placeholders::_1));
    RegisterAdministrationServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterAdministrationMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<RiskParametersMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind(&AdministrationClient::OnRiskParametersMessage, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Beam::Services::AddMessageSlot<RiskStateMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind(&AdministrationClient::OnRiskStateMessage, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  template<typename B>
  AdministrationClient<B>::~AdministrationClient() {
    Close();
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadAccountsByRoles(AccountRoles roles) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountsByRolesService>(roles);
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadAdministratorsRootEntry() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAdministratorsRootEntryService>();
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadServicesRootEntry() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadServicesRootEntryService>();
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadTradingGroupsRootEntry() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadTradingGroupsRootEntryService>();
  }

  template<typename B>
  bool AdministrationClient<B>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<CheckAdministratorService>(account);
  }

  template<typename B>
  AccountRoles AdministrationClient<B>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountRolesService>(account);
  }

  template<typename B>
  AccountRoles AdministrationClient<B>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSupervisedAccountRolesService>(
      parent, child);
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadParentTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadParentTradingGroupService>(account);
  }

  template<typename B>
  AccountIdentity AdministrationClient<B>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountIdentityService>(account);
  }

  template<typename B>
  void AdministrationClient<B>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreAccountIdentityService>(account,
      identity);
  }

  template<typename B>
  TradingGroup AdministrationClient<B>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadTradingGroupService>(directory);
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadManagedTradingGroupsService>(
      account);
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadAdministrators() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAdministratorsService>();
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadServices() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadServicesService>();
  }

  template<typename B>
  MarketDataService::EntitlementDatabase
      AdministrationClient<B>::LoadEntitlements() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadEntitlementsService>();
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountEntitlementsService>(
      account);
  }

  template<typename B>
  void AdministrationClient<B>::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreEntitlementsService>(account,
      entitlements);
  }

  template<typename B>
  const Beam::Publisher<RiskService::RiskParameters>&
      AdministrationClient<B>::GetRiskParametersPublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_riskParameterPublishers.GetOrInsert(account,
      [&] {
        auto publisher = std::make_shared<RiskParameterPublisher>();
        m_tasks.Push(
          [=] {
            try {
              auto client = m_clientHandler.GetClient();
              auto parameters =
                client->template SendRequest<MonitorRiskParametersService>(
                account);
              publisher->Push(parameters);
            } catch(const std::exception&) {
              publisher->Break(std::current_exception());
            }
          });
        return publisher;
      });
  }

  template<typename B>
  void AdministrationClient<B>::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreRiskParametersService>(account,
      riskParameters);
  }

  template<typename B>
  const Beam::Publisher<RiskService::RiskState>&
      AdministrationClient<B>::GetRiskStatePublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_riskStatePublishers.GetOrInsert(account,
      [&] {
        auto publisher = std::make_shared<RiskStatePublisher>();
        m_tasks.Push(
          [=] {
            try {
              auto client = m_clientHandler.GetClient();
              auto state = client->template SendRequest<
                MonitorRiskStateService>(account);
              publisher->Push(state);
            } catch(const std::exception&) {
              publisher->Break(std::current_exception());
            }
          });
        return publisher;
      });
  }

  template<typename B>
  void AdministrationClient<B>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreRiskStateService>(account, riskState);
  }

  template<typename B>
  AccountModificationRequest
      AdministrationClient<B>::LoadAccountModificationRequest(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountModificationRequestService>(
      id);
  }

  template<typename B>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient<B>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id id, int maxCount) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadAccountModificationRequestIdsService>(account, id, maxCount);
  }

  template<typename B>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient<B>::LoadManagedAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id id, int maxCount) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadManagedAccountModificationRequestIdsService>(account, id, maxCount);
  }

  template<typename B>
  EntitlementModification AdministrationClient<B>::LoadEntitlementModification(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadEntitlementModificationService>(id);
  }

  template<typename B>
  AccountModificationRequest
      AdministrationClient<B>::SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const EntitlementModification& modification, const Message& comment) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      SubmitEntitlementModificationRequestService>(account, modification,
      comment);
  }

  template<typename B>
  RiskModification AdministrationClient<B>::LoadRiskModification(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadRiskModificationService>(id);
  }

  template<typename B>
  AccountModificationRequest
      AdministrationClient<B>::SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskModification& modification, const Message& comment) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<SubmitRiskModificationRequestService>(
      account, modification, comment);
  }

  template<typename B>
  AccountModificationRequest::Update
      AdministrationClient<B>::LoadAccountModificationRequestStatus(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadAccountModificationRequestStatusService>(id);
  }

  template<typename B>
  AccountModificationRequest::Update
      AdministrationClient<B>::ApproveAccountModificationRequest(
      AccountModificationRequest::Id id, const Message& comment) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      ApproveAccountModificationRequestService>(id, comment);
  }

  template<typename B>
  AccountModificationRequest::Update
      AdministrationClient<B>::RejectAccountModificationRequest(
      AccountModificationRequest::Id id, const Message& comment) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      RejectAccountModificationRequestService>(id, comment);
  }

  template<typename B>
  Message AdministrationClient<B>::LoadMessage(Message::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadMessageService>(id);
  }

  template<typename B>
  std::vector<Message::Id> AdministrationClient<B>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadMessageIdsService>(id);
  }

  template<typename B>
  Message AdministrationClient<B>::SendAccountModificationRequestMessage(
      AccountModificationRequest::Id id, const Message& message) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      SendAccountModificationRequestMessageService>(id, message);
  }

  template<typename B>
  void AdministrationClient<B>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename B>
  void AdministrationClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename B>
  void AdministrationClient<B>::Shutdown() {
    m_clientHandler.Close();
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename B>
  void AdministrationClient<B>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.Push(
      [=] {
        RecoverRiskParameters(*client);
        RecoverRiskState(*client);
      });
  }

  template<typename B>
  void AdministrationClient<B>::RecoverRiskParameters(
      ServiceProtocolClient& client) {
    auto currentRiskParameterEntries =
      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      std::shared_ptr<RiskParameterPublisher>>>();
    m_riskParameterPublishers.With(
      [&] (auto& riskParameterPublishers) {
        for(auto& publisher : riskParameterPublishers) {
          currentRiskParameterEntries.emplace_back(
            publisher.first, publisher.second);
        }
      });
    for(auto& entry : currentRiskParameterEntries) {
      auto& publisher = std::get<1>(entry);
      try {
        auto parameters =
          client.template SendRequest<MonitorRiskParametersService>(
          std::get<0>(entry));
        auto publish = bool();
        publisher->WithSnapshot(
          [&] (auto snapshot) {
            if(!snapshot.is_initialized() || *snapshot != parameters) {
              publish = true;
            } else {
              publish = false;
            }
          });
        if(publish) {
          publisher->Push(parameters);
        }
      } catch(const std::exception&) {
        publisher->Break(std::current_exception());
      }
    }
  }

  template<typename B>
  void AdministrationClient<B>::RecoverRiskState(
      ServiceProtocolClient& client) {
    auto currentRiskStateEntries =
      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      std::shared_ptr<RiskStatePublisher>>>();
    m_riskStatePublishers.With(
      [&] (auto& riskStatePublishers) {
        for(auto& publisher : riskStatePublishers) {
          currentRiskStateEntries.emplace_back(
            publisher.first, publisher.second);
        }
      });
    for(auto& entry : currentRiskStateEntries) {
      auto& publisher = std::get<1>(entry);
      try {
        auto state = client.template SendRequest<MonitorRiskStateService>(
          std::get<0>(entry));
        auto publish = bool();
        publisher->WithSnapshot(
          [&] (auto snapshot) {
            if(!snapshot.is_initialized() || *snapshot != state) {
              publish = true;
            } else {
              publish = false;
            }
          });
        if(publish) {
          publisher->Push(state);
        }
      } catch(const std::exception&) {
        publisher->Break(std::current_exception());
      }
    }
  }

  template<typename B>
  void AdministrationClient<B>::OnRiskParametersMessage(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_tasks.Push(
      [=] {
        auto publisher = m_riskParameterPublishers.FindValue(account);
        if(!publisher.is_initialized()) {
          return;
        }
        (*publisher)->Push(riskParameters);
      });
  }

  template<typename B>
  void AdministrationClient<B>::OnRiskStateMessage(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      RiskService::RiskState riskState) {
    m_tasks.Push(
      [=] {
        auto publisher = m_riskStatePublishers.FindValue(account);
        if(!publisher.is_initialized()) {
          return;
        }
        (*publisher)->Push(riskState);
      });
  }
}

#endif
