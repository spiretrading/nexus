#ifndef NEXUS_TEST_ADMINISTRATION_CLIENT_HPP
#define NEXUS_TEST_ADMINISTRATION_CLIENT_HPP
#include <variant>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"

namespace Nexus::Tests {

  /**
   * Implements an AdministrationClient for testing by pushing all operations
   * performed onto a queue.
   */
  class TestAdministrationClient {
    public:

      /** Records a call to load_accounts_by_roles(). */
      struct LoadAccountsByRolesOperation {
        AccountRoles m_roles;
        Beam::Tests::ServiceResult<std::vector<Beam::DirectoryEntry>> m_result;
      };

      /** Records a call to load_administrators_root_entry(). */
      struct LoadAdministratorsRootEntryOperation {
        Beam::Tests::ServiceResult<Beam::DirectoryEntry> m_result;
      };

      /** Records a call to load_services_root_entry(). */
      struct LoadServicesRootEntryOperation {
        Beam::Tests::ServiceResult<Beam::DirectoryEntry> m_result;
      };

      /** Records a call to load_trading_groups_root_entry(). */
      struct LoadTradingGroupsRootEntryOperation {
        Beam::Tests::ServiceResult<Beam::DirectoryEntry> m_result;
      };

      /** Records a call to check_administrator(). */
      struct CheckAdministratorOperation {
        Beam::DirectoryEntry m_account;
        Beam::Tests::ServiceResult<bool> m_result;
      };

      /** Records a call to load_account_roles(). */
      struct LoadAccountRolesOperation {
        Beam::DirectoryEntry m_account;
        Beam::Tests::ServiceResult<AccountRoles> m_result;
      };

      /** Records a call to load_account_roles(). */
      struct LoadParentChildAccountRolesOperation {
        Beam::DirectoryEntry m_parent;
        Beam::DirectoryEntry m_child;
        Beam::Tests::ServiceResult<AccountRoles> m_result;
      };

      /** Records a call to load_parent_trading_group(). */
      struct LoadParentTradingGroupOperation {
        Beam::DirectoryEntry m_account;
        Beam::Tests::ServiceResult<Beam::DirectoryEntry> m_result;
      };

      /** Records a call to load_identity(). */
      struct LoadIdentityOperation {
        Beam::DirectoryEntry m_account;
        Beam::Tests::ServiceResult<AccountIdentity> m_result;
      };

      /** Records a call to store_identity(). */
      struct StoreIdentityOperation {
        Beam::DirectoryEntry m_account;
        AccountIdentity m_identity;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to load_trading_group(). */
      struct LoadTradingGroupOperation {
        Beam::DirectoryEntry m_directory;
        Beam::Tests::ServiceResult<TradingGroup> m_result;
      };

      /** Records a call to load_managed_trading_groups(). */
      struct LoadManagedTradingGroupsOperation {
        Beam::DirectoryEntry m_account;
        Beam::Tests::ServiceResult<std::vector<Beam::DirectoryEntry>> m_result;
      };

      /** Records a call to load_administrators(). */
      struct LoadAdministratorsOperation {
        Beam::Tests::ServiceResult<std::vector<Beam::DirectoryEntry>> m_result;
      };

      /** Records a call to load_services(). */
      struct LoadServicesOperation {
        Beam::Tests::ServiceResult<std::vector<Beam::DirectoryEntry>> m_result;
      };

      /** Records a call to load_entitlements(). */
      struct LoadEntitlementsOperation {
        Beam::Tests::ServiceResult<EntitlementDatabase> m_result;
      };

      /** Records a call to load_entitlements(). */
      struct LoadAccountEntitlementsOperation {
        Beam::DirectoryEntry m_account;
        Beam::Tests::ServiceResult<std::vector<Beam::DirectoryEntry>> m_result;
      };

      /** Records a call to store_entitlements(). */
      struct StoreEntitlementsOperation {
        Beam::DirectoryEntry m_account;
        std::vector<Beam::DirectoryEntry> m_entitlements;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to get_risk_parameters_publisher(). */
      struct MonitorRiskParametersOperation {
        Beam::DirectoryEntry m_account;
        Beam::QueueWriterPublisher<RiskParameters> m_queue;
      };

      /** Records a call to store_risk_parameters(). */
      struct StoreRiskParametersOperation {
        Beam::DirectoryEntry m_account;
        RiskParameters m_parameters;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to get_risk_state_publisher(). */
      struct MonitorRiskStateOperation {
        Beam::DirectoryEntry m_account;
        Beam::QueueWriterPublisher<RiskState> m_queue;
      };

      /** Records a call to store_risk_state(). */
      struct StoreRiskStateOperation {
        Beam::DirectoryEntry m_account;
        RiskState m_state;
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to load_account_modification_request(). */
      struct LoadAccountModificationRequestOperation {
        AccountModificationRequest::Id m_id;
        Beam::Tests::ServiceResult<AccountModificationRequest> m_result;
      };

      /** Records a call to load_account_modification_request_ids(). */
      struct LoadAccountModificationRequestIdsOperation {
        Beam::DirectoryEntry m_account;
        AccountModificationRequest::Id m_start_id;
        int m_max_count;
        Beam::Tests::ServiceResult<std::vector<AccountModificationRequest::Id>>
          m_result;
      };

      /** Records a call to load_managed_account_modification_request_ids(). */
      struct LoadManagedAccountModificationRequestIdsOperation {
        Beam::DirectoryEntry m_account;
        AccountModificationRequest::Id m_start_id;
        int m_max_count;
        Beam::Tests::ServiceResult<std::vector<AccountModificationRequest::Id>>
          m_result;
      };

      /** Records a call to load_entitlement_modification(). */
      struct LoadEntitlementModificationOperation {
        AccountModificationRequest::Id m_id;
        Beam::Tests::ServiceResult<EntitlementModification> m_result;
      };

      /**
       * Records a call to submit_account_modification_request() for
       * entitlements.
       */
      struct SubmitEntitlementModificationRequestOperation {
        Beam::DirectoryEntry m_account;
        EntitlementModification m_modification;
        Message m_comment;
        Beam::Tests::ServiceResult<AccountModificationRequest> m_result;
      };

      /** Records a call to load_risk_modification(). */
      struct LoadRiskModificationOperation {
        AccountModificationRequest::Id m_id;
        Beam::Tests::ServiceResult<RiskModification> m_result;
      };

      /** Records a call to submit_account_modification_request() for risk. */
      struct SubmitRiskModificationRequestOperation {
        Beam::DirectoryEntry m_account;
        RiskModification m_modification;
        Message m_comment;
        Beam::Tests::ServiceResult<AccountModificationRequest> m_result;
      };

      /** Records a call to load_account_modification_request_status(). */
      struct LoadAccountModificationRequestStatusOperation {
        AccountModificationRequest::Id m_id;
        Beam::Tests::ServiceResult<AccountModificationRequest::Update> m_result;
      };

      /** Records a call to approve_account_modification_request(). */
      struct ApproveAccountModificationRequestOperation {
        AccountModificationRequest::Id m_id;
        Message m_comment;
        Beam::Tests::ServiceResult<AccountModificationRequest::Update> m_result;
      };

      /** Records a call to reject_account_modification_request(). */
      struct RejectAccountModificationRequestOperation {
        AccountModificationRequest::Id m_id;
        Message m_comment;
        Beam::Tests::ServiceResult<AccountModificationRequest::Update> m_result;
      };

      /** Records a call to load_message(). */
      struct LoadMessageOperation {
        Message::Id m_id;
        Beam::Tests::ServiceResult<Message> m_result;
      };

      /** Records a call to load_message_ids(). */
      struct LoadMessageIdsOperation {
        AccountModificationRequest::Id m_id;
        Beam::Tests::ServiceResult<std::vector<Message::Id>> m_result;
      };

      /** Records a call to send_account_modification_request_message(). */
      struct SendAccountModificationRequestMessageOperation {
        AccountModificationRequest::Id m_id;
        Message m_message;
        Beam::Tests::ServiceResult<Message> m_result;
      };

      /**
       * A variant covering all possible TestAdministrationClient operations.
       */
      using Operation = std::variant<LoadAccountsByRolesOperation,
        LoadAdministratorsRootEntryOperation, LoadServicesRootEntryOperation,
        LoadTradingGroupsRootEntryOperation, CheckAdministratorOperation,
        LoadAccountRolesOperation, LoadParentChildAccountRolesOperation,
        LoadParentTradingGroupOperation, LoadIdentityOperation,
        StoreIdentityOperation, LoadTradingGroupOperation,
        LoadManagedTradingGroupsOperation, LoadAdministratorsOperation,
        LoadServicesOperation, LoadEntitlementsOperation,
        LoadAccountEntitlementsOperation, StoreEntitlementsOperation,
        MonitorRiskParametersOperation, StoreRiskParametersOperation,
        MonitorRiskStateOperation, StoreRiskStateOperation,
        LoadAccountModificationRequestOperation,
        LoadAccountModificationRequestIdsOperation,
        LoadManagedAccountModificationRequestIdsOperation,
        LoadEntitlementModificationOperation,
        SubmitEntitlementModificationRequestOperation,
        LoadRiskModificationOperation,
        SubmitRiskModificationRequestOperation,
        LoadAccountModificationRequestStatusOperation,
        ApproveAccountModificationRequestOperation,
        RejectAccountModificationRequestOperation, LoadMessageOperation,
        LoadMessageIdsOperation,
        SendAccountModificationRequestMessageOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestAdministrationClient.
       * @param operations The queue to push all operations on.
       */
      explicit TestAdministrationClient(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept;

      ~TestAdministrationClient();

      std::vector<Beam::DirectoryEntry> load_accounts_by_roles(
        AccountRoles roles);
      Beam::DirectoryEntry load_administrators_root_entry();
      Beam::DirectoryEntry load_services_root_entry();
      Beam::DirectoryEntry load_trading_groups_root_entry();
      bool check_administrator(const Beam::DirectoryEntry& account);
      AccountRoles load_account_roles(const Beam::DirectoryEntry& account);
      AccountRoles load_account_roles(const Beam::DirectoryEntry& parent,
        const Beam::DirectoryEntry& child);
      Beam::DirectoryEntry load_parent_trading_group(
        const Beam::DirectoryEntry& account);
      AccountIdentity load_identity(const Beam::DirectoryEntry& account);
      void store(
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);
      TradingGroup load_trading_group(const Beam::DirectoryEntry& directory);
      std::vector<Beam::DirectoryEntry> load_managed_trading_groups(
        const Beam::DirectoryEntry& account);
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
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_queue;

      TestAdministrationClient(const TestAdministrationClient&) = delete;
      TestAdministrationClient& operator =(
        const TestAdministrationClient&) = delete;
  };

  inline TestAdministrationClient::TestAdministrationClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_queue(std::move(operations)) {}

  inline TestAdministrationClient::~TestAdministrationClient() {
    close();
  }

  inline std::vector<Beam::DirectoryEntry>
      TestAdministrationClient::load_accounts_by_roles(AccountRoles roles) {
    return m_queue.append_result<LoadAccountsByRolesOperation,
      std::vector<Beam::DirectoryEntry>>(std::move(roles));
  }

  inline Beam::DirectoryEntry
      TestAdministrationClient::load_administrators_root_entry() {
    return m_queue.append_result<
      LoadAdministratorsRootEntryOperation, Beam::DirectoryEntry>();
  }

  inline Beam::DirectoryEntry
      TestAdministrationClient::load_services_root_entry() {
    return m_queue.append_result<
      LoadServicesRootEntryOperation, Beam::DirectoryEntry>();
  }

  inline Beam::DirectoryEntry
      TestAdministrationClient::load_trading_groups_root_entry() {
    return m_queue.append_result<
      LoadTradingGroupsRootEntryOperation, Beam::DirectoryEntry>();
  }

  inline bool TestAdministrationClient::check_administrator(
      const Beam::DirectoryEntry& account) {
    return m_queue.append_result<CheckAdministratorOperation, bool>(account);
  }

  inline AccountRoles TestAdministrationClient::load_account_roles(
      const Beam::DirectoryEntry& account) {
    return m_queue.append_result<LoadAccountRolesOperation, AccountRoles>(
      account);
  }

  inline AccountRoles TestAdministrationClient::load_account_roles(
      const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child) {
    return m_queue.append_result<
      LoadParentChildAccountRolesOperation, AccountRoles>(parent, child);
  }

  inline Beam::DirectoryEntry
      TestAdministrationClient::load_parent_trading_group(
        const Beam::DirectoryEntry& account) {
    return m_queue.append_result<
      LoadParentTradingGroupOperation, Beam::DirectoryEntry>(account);
  }

  inline AccountIdentity TestAdministrationClient::load_identity(
      const Beam::DirectoryEntry& account) {
    return m_queue.append_result<LoadIdentityOperation, AccountIdentity>(
      account);
  }

  inline void TestAdministrationClient::store(
      const Beam::DirectoryEntry& account, const AccountIdentity& identity) {
    return m_queue.append_result<StoreIdentityOperation, void>(
      account, identity);
  }

  inline TradingGroup TestAdministrationClient::load_trading_group(
      const Beam::DirectoryEntry& directory) {
    return m_queue.append_result<LoadTradingGroupOperation, TradingGroup>(
      directory);
  }

  inline std::vector<Beam::DirectoryEntry>
      TestAdministrationClient::load_managed_trading_groups(
        const Beam::DirectoryEntry& account) {
    return m_queue.append_result<LoadManagedTradingGroupsOperation,
      std::vector<Beam::DirectoryEntry>>(account);
  }

  inline std::vector<Beam::DirectoryEntry>
      TestAdministrationClient::load_administrators() {
    return m_queue.append_result<
      LoadAdministratorsOperation, std::vector<Beam::DirectoryEntry>>();
  }

  inline std::vector<Beam::DirectoryEntry>
      TestAdministrationClient::load_services() {
    return m_queue.append_result<
      LoadServicesOperation, std::vector<Beam::DirectoryEntry>>();
  }

  inline EntitlementDatabase TestAdministrationClient::load_entitlements() {
    return m_queue.append_result<
      LoadEntitlementsOperation, EntitlementDatabase>();
  }

  inline std::vector<Beam::DirectoryEntry>
      TestAdministrationClient::load_entitlements(
        const Beam::DirectoryEntry& account) {
    return m_queue.append_result<LoadAccountEntitlementsOperation,
      std::vector<Beam::DirectoryEntry>>(account);
  }

  inline void TestAdministrationClient::store_entitlements(
      const Beam::DirectoryEntry& account,
      const std::vector<Beam::DirectoryEntry>& entitlements) {
    return m_queue.append_result<StoreEntitlementsOperation, void>(
      account, entitlements);
  }

  inline const Beam::Publisher<RiskParameters>&
      TestAdministrationClient::get_risk_parameters_publisher(
        const Beam::DirectoryEntry& account) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<MonitorRiskParametersOperation>, account);
    m_queue.append_queue<MonitorRiskParametersOperation>(operation);
    return std::get<MonitorRiskParametersOperation>(*operation).m_queue;
  }

  inline void TestAdministrationClient::store(
      const Beam::DirectoryEntry& account, const RiskParameters& parameters) {
    return m_queue.append_result<StoreRiskParametersOperation, void>(
      account, parameters);
  }

  inline const Beam::Publisher<RiskState>&
      TestAdministrationClient::get_risk_state_publisher(
        const Beam::DirectoryEntry& account) {
    auto operation = std::make_shared<Operation>(
      std::in_place_type<MonitorRiskStateOperation>, account);
    m_queue.append_queue<MonitorRiskStateOperation>(operation);
    return std::get<MonitorRiskStateOperation>(*operation).m_queue;
  }

  inline void TestAdministrationClient::store(
      const Beam::DirectoryEntry& account, const RiskState& state) {
    return m_queue.append_result<StoreRiskStateOperation, void>(
      account, state);
  }

  inline AccountModificationRequest
      TestAdministrationClient::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return m_queue.append_result<
      LoadAccountModificationRequestOperation, AccountModificationRequest>(id);
  }

  inline std::vector<AccountModificationRequest::Id>
      TestAdministrationClient::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_queue.append_result<LoadAccountModificationRequestIdsOperation,
      std::vector<AccountModificationRequest::Id>>(
        account, start_id, max_count);
  }

  inline std::vector<AccountModificationRequest::Id>
      TestAdministrationClient::load_managed_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return m_queue.append_result<
      LoadManagedAccountModificationRequestIdsOperation,
      std::vector<AccountModificationRequest::Id>>(
        account, start_id, max_count);
  }

  inline EntitlementModification
      TestAdministrationClient::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    return m_queue.append_result<
      LoadEntitlementModificationOperation, EntitlementModification>(id);
  }

  inline AccountModificationRequest TestAdministrationClient::submit(
      const Beam::DirectoryEntry& account,
      const EntitlementModification& modification, const Message& comment) {
    return m_queue.append_result<SubmitEntitlementModificationRequestOperation,
      AccountModificationRequest>(account, modification, comment);
  }

  inline RiskModification TestAdministrationClient::load_risk_modification(
      AccountModificationRequest::Id id) {
    return m_queue.append_result<
      LoadRiskModificationOperation, RiskModification>(id);
  }

  inline AccountModificationRequest TestAdministrationClient::submit(
      const Beam::DirectoryEntry& account, const RiskModification& modification,
      const Message& comment) {
    return m_queue.append_result<SubmitRiskModificationRequestOperation,
      AccountModificationRequest>(account, modification, comment);
  }

  inline AccountModificationRequest::Update
      TestAdministrationClient::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return m_queue.append_result<LoadAccountModificationRequestStatusOperation,
      AccountModificationRequest::Update>(id);
  }

  inline AccountModificationRequest::Update
      TestAdministrationClient::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_queue.append_result<ApproveAccountModificationRequestOperation,
      AccountModificationRequest::Update>(id, comment);
  }

  inline AccountModificationRequest::Update
      TestAdministrationClient::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return m_queue.append_result<RejectAccountModificationRequestOperation,
      AccountModificationRequest::Update>(id, comment);
  }

  inline Message TestAdministrationClient::load_message(Message::Id id) {
    return m_queue.append_result<LoadMessageOperation, Message>(id);
  }

  inline std::vector<Message::Id> TestAdministrationClient::load_message_ids(
      AccountModificationRequest::Id id) {
    return m_queue.append_result<
      LoadMessageIdsOperation, std::vector<Message::Id>>(id);
  }

  inline Message TestAdministrationClient::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    return m_queue.append_result<
      SendAccountModificationRequestMessageOperation, Message>(id, message);
  }

  inline void TestAdministrationClient::close() {
    m_queue.close();
  }
}

#endif
