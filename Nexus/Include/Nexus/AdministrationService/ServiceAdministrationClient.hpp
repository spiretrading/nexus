#ifndef NEXUS_SERVICE_ADMINISTRATION_CLIENT_HPP
#define NEXUS_SERVICE_ADMINISTRATION_CLIENT_HPP
#include <vector>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/lexical_cast.hpp>
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
  class AdministrationClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs an AdministrationClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit AdministrationClient(BF&& clientBuilder);

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
        LoadManagedTradingGroups(
          const Beam::ServiceLocator::DirectoryEntry& account);

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

      AdministrationClient(const AdministrationClient&) = delete;
      AdministrationClient& operator =(const AdministrationClient&) = delete;
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

  /**
   * Loads an account's RiskParameters.
   * @param client The AdministrationClient used to load the parameters.
   * @param account The account whose parameters are to be loaded.
   * @return The <i>account</i>'s RiskParameters.
   */
  template<typename Client>
  RiskService::RiskParameters LoadRiskParameters(Client& client,
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto queue = std::make_shared<
      Beam::StateQueue<RiskService::RiskParameters>>();
    client.GetRiskParametersPublisher(account).Monitor(queue);
    return queue->Pop();
  }

  template<typename B>
  template<typename BF>
  AdministrationClient<B>::AdministrationClient(BF&& clientBuilder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_clientHandler(std::forward<BF>(clientBuilder),
              std::bind_front(&AdministrationClient::OnReconnect, this)) {
    RegisterAdministrationServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterAdministrationMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<RiskParametersMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind_front(&AdministrationClient::OnRiskParametersMessage, this));
    Beam::Services::AddMessageSlot<RiskStateMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind_front(&AdministrationClient::OnRiskStateMessage, this));
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the administration server."));
  }

  template<typename B>
  AdministrationClient<B>::~AdministrationClient() {
    Close();
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadAccountsByRoles(AccountRoles roles) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadAccountsByRolesService>(roles);
    }, "Failed to load accounts by roles: " +
      boost::lexical_cast<std::string>(roles));
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadAdministratorsRootEntry() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadAdministratorsRootEntryService>();
    }, "Failed to load administrators root entry.");
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadServicesRootEntry() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadServicesRootEntryService>();
    }, "Failed to load services root entry.");
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadTradingGroupsRootEntry() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadTradingGroupsRootEntryService>();
    }, "Failed to load trading groups root entry.");
  }

  template<typename B>
  bool AdministrationClient<B>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<CheckAdministratorService>(account);
    }, "Failed to check administrator: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountRoles AdministrationClient<B>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadAccountRolesService>(account);
    }, "Failed to load account roles: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountRoles AdministrationClient<B>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadSupervisedAccountRolesService>(
        parent, child);
    }, "Failed to load account roles: " +
      boost::lexical_cast<std::string>(parent) + ", " +
      boost::lexical_cast<std::string>(child));
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      AdministrationClient<B>::LoadParentTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadParentTradingGroupService>(
        account);
    }, "Failed to load parent trading group: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountIdentity AdministrationClient<B>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadAccountIdentityService>(account);
    }, "Failed to load identity: " + boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void AdministrationClient<B>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<StoreAccountIdentityService>(
        account, identity);
    }, "Failed to store identity: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  TradingGroup AdministrationClient<B>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadTradingGroupService>(directory);
    }, "Failed to load trading group: " +
      boost::lexical_cast<std::string>(directory));
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadManagedTradingGroupsService>(
        account);
    }, "Failed to load managed trading groups: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadAdministrators() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadAdministratorsService>();
    }, "Failed to load administrators.");
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadServices() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadServicesService>();
    }, "Failed to load services.");
  }

  template<typename B>
  MarketDataService::EntitlementDatabase
      AdministrationClient<B>::LoadEntitlements() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadEntitlementsService>();
    }, "Failed to load entitlements.");
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<B>::LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadAccountEntitlementsService>(
        account);
    }, "Failed to load entitlements: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void AdministrationClient<B>::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<StoreEntitlementsService>(account,
        entitlements);
    }, "Failed to store entitlements: " +
      boost::lexical_cast<std::string>(account) +
      boost::lexical_cast<std::string>(Beam::Stream(entitlements)));
  }

  template<typename B>
  const Beam::Publisher<RiskService::RiskParameters>&
      AdministrationClient<B>::GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_riskParameterPublishers.GetOrInsert(account, [&] {
      auto publisher = std::make_shared<RiskParameterPublisher>();
      m_tasks.Push([=, this] {
        try {
          auto client = m_clientHandler.GetClient();
          auto parameters =
            client->template SendRequest<MonitorRiskParametersService>(account);
          publisher->Push(parameters);
        } catch(const std::exception&) {
          publisher->Break(Beam::Services::MakeNestedServiceException(
            "Failed to monitor risk parameters: " +
            boost::lexical_cast<std::string>(account)));
        }
      });
      return publisher;
    });
  }

  template<typename B>
  void AdministrationClient<B>::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<StoreRiskParametersService>(account,
        riskParameters);
    }, "Failed to store risk parameters: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(riskParameters));
  }

  template<typename B>
  const Beam::Publisher<RiskService::RiskState>&
      AdministrationClient<B>::GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_riskStatePublishers.GetOrInsert(account, [&] {
      auto publisher = std::make_shared<RiskStatePublisher>();
      m_tasks.Push([=, this] {
        try {
          auto client = m_clientHandler.GetClient();
          auto state = client->template SendRequest<MonitorRiskStateService>(
            account);
          publisher->Push(state);
        } catch(const std::exception&) {
          publisher->Break(Beam::Services::MakeNestedServiceException(
            "Failed to monitor risk state: " + boost::lexical_cast<std::string>(
              account)));
        }
      });
      return publisher;
    });
  }

  template<typename B>
  void AdministrationClient<B>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<StoreRiskStateService>(account, riskState);
    }, "Failed to store risk state: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(riskState));
  }

  template<typename B>
  AccountModificationRequest
      AdministrationClient<B>::LoadAccountModificationRequest(
        AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        LoadAccountModificationRequestService>(id);
    }, "Failed to load account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient<B>::LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id id, int maxCount) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        LoadAccountModificationRequestIdsService>(account, id, maxCount);
    }, "Failed to load account modification request ids: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(id) + ", " +
      boost::lexical_cast<std::string>(maxCount));
  }

  template<typename B>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient<B>::LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id id, int maxCount) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        LoadManagedAccountModificationRequestIdsService>(account, id, maxCount);
    }, "Failed to load managed account modification request ids: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(id) + ", " +
      boost::lexical_cast<std::string>(maxCount));
  }

  template<typename B>
  EntitlementModification AdministrationClient<B>::LoadEntitlementModification(
      AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadEntitlementModificationService>(
        id);
    }, "Failed to load entitlement modification: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest
      AdministrationClient<B>::SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        SubmitEntitlementModificationRequestService>(account, modification,
        comment);
    }, "Failed to submit account modification request: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  RiskModification AdministrationClient<B>::LoadRiskModification(
      AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadRiskModificationService>(id);
    }, "Failed to load risk modification: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest
      AdministrationClient<B>::SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<SubmitRiskModificationRequestService>(
        account, modification, comment);
    }, "Failed to submit account modification request: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountModificationRequest::Update
      AdministrationClient<B>::LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        LoadAccountModificationRequestStatusService>(id);
    }, "Failed to load account modification request status: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest::Update
      AdministrationClient<B>::ApproveAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        ApproveAccountModificationRequestService>(id, comment);
    }, "Failed to approve account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest::Update
      AdministrationClient<B>::RejectAccountModificationRequest(
        AccountModificationRequest::Id id, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        RejectAccountModificationRequestService>(id, comment);
    }, "Failed to reject account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  Message AdministrationClient<B>::LoadMessage(Message::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadMessageService>(id);
    }, "Failed to load message: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  std::vector<Message::Id> AdministrationClient<B>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<LoadMessageIdsService>(id);
    }, "Failed to load message ids: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  Message AdministrationClient<B>::SendAccountModificationRequestMessage(
      AccountModificationRequest::Id id, const Message& message) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        SendAccountModificationRequestMessageService>(id, message);
    }, "Failed to send account modification request message: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void AdministrationClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_clientHandler.Close();
    m_riskStatePublishers.Clear();
    m_riskParameterPublishers.Clear();
    m_openState.Close();
  }

  template<typename B>
  void AdministrationClient<B>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.Push([=, this] {
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
    m_riskParameterPublishers.With([&] (auto& riskParameterPublishers) {
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
        if(publisher->GetSnapshot() != parameters) {
          publisher->Push(parameters);
        }
      } catch(const std::exception&) {
        publisher->Break(Beam::Services::MakeNestedServiceException(
          "Failed to recover risk parameters."));
      }
    }
  }

  template<typename B>
  void AdministrationClient<B>::RecoverRiskState(
      ServiceProtocolClient& client) {
    auto currentRiskStateEntries =
      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskStatePublisher>>>();
    m_riskStatePublishers.With([&] (auto& riskStatePublishers) {
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
        if(publisher->GetSnapshot() != state) {
          publisher->Push(state);
        }
      } catch(const std::exception&) {
        publisher->Break(Beam::Services::MakeNestedServiceException(
          "Failed to recover risk state."));
      }
    }
  }

  template<typename B>
  void AdministrationClient<B>::OnRiskParametersMessage(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_tasks.Push([=, this] {
      if(auto publisher = m_riskParameterPublishers.FindValue(account)) {
        try {
          (*publisher)->Push(riskParameters);
        } catch(const Beam::PipeBrokenException&) {
          m_riskParameterPublishers.Erase(account);
        }
      }
    });
  }

  template<typename B>
  void AdministrationClient<B>::OnRiskStateMessage(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      RiskService::RiskState riskState) {
    m_tasks.Push([=, this] {
      if(auto publisher = m_riskStatePublishers.FindValue(account)) {
        try {
          (*publisher)->Push(riskState);
        } catch(const Beam::PipeBrokenException&) {
          m_riskStatePublishers.Erase(account);
        }
      }
    });
  }
}

#endif
