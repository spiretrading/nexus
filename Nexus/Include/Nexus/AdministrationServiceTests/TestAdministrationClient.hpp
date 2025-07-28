#ifndef NEXUS_TEST_ADMINISTRATION_CLIENT_HPP
#define NEXUS_TEST_ADMINISTRATION_CLIENT_HPP
#include <variant>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"

namespace Nexus::AdministrationService::Tests {

  /**
   * Implements an AdministrationClient for testing by pushing all operations
   * performed onto a queue.
   */
  class TestAdministrationClient {
    public:

      /** Records a call to load_accounts_by_roles(). */
      struct LoadAccountsByRolesOperation {
        AccountRoles m_roles;
        Beam::Services::Tests::ServiceResult<
          std::vector<Beam::ServiceLocator::DirectoryEntry>> m_result;
      };

      /** Records a call to load_administrators_root_entry(). */
      struct LoadAdministratorsRootEntryOperation {
        Beam::Services::Tests::ServiceResult<
          Beam::ServiceLocator::DirectoryEntry> m_result;
      };

      /** Records a call to load_services_root_entry(). */
      struct LoadServicesRootEntryOperation {
        Beam::Services::Tests::ServiceResult<
          Beam::ServiceLocator::DirectoryEntry> m_result;
      };

      /** Records a call to load_trading_groups_root_entry(). */
      struct LoadTradingGroupsRootEntryOperation {
        Beam::Services::Tests::ServiceResult<
          Beam::ServiceLocator::DirectoryEntry> m_result;
      };

      /** Records a call to check_administrator(). */
      struct CheckAdministratorOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<bool> m_result;
      };

      /** Records a call to load_account_roles(). */
      struct LoadAccountRolesOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<AccountRoles> m_result;
      };

      /** Records a call to load_account_roles(). */
      struct LoadParentChildAccountRolesOperation {
        Beam::ServiceLocator::DirectoryEntry m_parent;
        Beam::ServiceLocator::DirectoryEntry m_child;
        Beam::Services::Tests::ServiceResult<AccountRoles> m_result;
      };

      /** Records a call to load_parent_trading_group(). */
      struct LoadParentTradingGroupOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<
          Beam::ServiceLocator::DirectoryEntry> m_result;
      };

      /** Records a call to load_identity(). */
      struct LoadIdentityOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<AccountIdentity> m_result;
      };

      /** Records a call to store_identity(). */
      struct StoreIdentityOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        AccountIdentity m_identity;
        Beam::Services::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to load_trading_group(). */
      struct LoadTradingGroupOperation {
        Beam::ServiceLocator::DirectoryEntry m_directory;
        Beam::Services::Tests::ServiceResult<TradingGroup> m_result;
      };

      /** Records a call to load_managed_trading_groups(). */
      struct LoadManagedTradingGroupsOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<
          std::vector<Beam::ServiceLocator::DirectoryEntry>> m_result;
      };

      /** Records a call to load_administrators(). */
      struct LoadAdministratorsOperation {
        Beam::Services::Tests::ServiceResult<
          std::vector<Beam::ServiceLocator::DirectoryEntry>> m_result;
      };

      /** Records a call to load_services(). */
      struct LoadServicesOperation {
        Beam::Services::Tests::ServiceResult<
          std::vector<Beam::ServiceLocator::DirectoryEntry>> m_result;
      };

      /** Records a call to load_entitlements(). */
      struct LoadEntitlementsOperation {
        Beam::Services::Tests::ServiceResult<
          MarketDataService::EntitlementDatabase> m_result;
      };

      /** Records a call to load_entitlements(). */
      struct LoadAccountEntitlementsOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<
          std::vector<Beam::ServiceLocator::DirectoryEntry>> m_result;
      };

      /** Records a call to store_entitlements(). */
      struct StoreEntitlementsOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        std::vector<Beam::ServiceLocator::DirectoryEntry> m_entitlements;
        Beam::Services::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to get_risk_parameters_publisher(). */
      struct MonitorRiskParametersOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<RiskService::RiskParameters>
          m_result;
      };

      /** Records a call to store_risk_parameters(). */
      struct StoreRiskParametersOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        RiskService::RiskParameters m_risk_parameters;
        Beam::Services::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to load_risk_state(). */
      struct LoadRiskStateOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        Beam::Services::Tests::ServiceResult<RiskService::RiskState> m_result;
      };

      /** Records a call to store_risk_state(). */
      struct StoreRiskStateOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        RiskService::RiskState m_risk_state;
        Beam::Services::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to load_account_modification_request(). */
      struct LoadAccountModificationRequestOperation {
        AccountModificationRequest::Id m_id;
        Beam::Services::Tests::ServiceResult<AccountModificationRequest>
          m_result;
      };

      /** Records a call to load_account_modification_request_ids(). */
      struct LoadAccountModificationRequestIdsOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        AccountModificationRequest::Id m_start_id;
        int m_max_count;
        Beam::Services::Tests::ServiceResult<
          std::vector<AccountModificationRequest::Id>> m_result;
      };

      /** Records a call to load_managed_account_modification_request_ids(). */
      struct LoadManagedAccountModificationRequestIdsOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        AccountModificationRequest::Id m_start_id;
        int m_max_count;
        Beam::Services::Tests::ServiceResult<
          std::vector<AccountModificationRequest::Id>> m_result;
      };

      /** Records a call to load_entitlement_modification(). */
      struct LoadEntitlementModificationOperation {
        AccountModificationRequest::Id m_id;
        Beam::Services::Tests::ServiceResult<EntitlementModification> m_result;
      };

      /**
       * Records a call to submit_account_modification_request() for
       * entitlements.
       */
      struct SubmitEntitlementModificationRequestOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        EntitlementModification m_modification;
        Message m_comment;
        Beam::Services::Tests::ServiceResult<AccountModificationRequest>
          m_result;
      };

      /** Records a call to load_risk_modification(). */
      struct LoadRiskModificationOperation {
        AccountModificationRequest::Id m_id;
        Beam::Services::Tests::ServiceResult<RiskModification> m_result;
      };

      /** Records a call to submit_account_modification_request() for risk. */
      struct SubmitRiskModificationRequestOperation {
        Beam::ServiceLocator::DirectoryEntry m_account;
        RiskModification m_modification;
        Message m_comment;
        Beam::Services::Tests::ServiceResult<AccountModificationRequest>
          m_result;
      };

      /** Records a call to load_account_modification_request_status(). */
      struct LoadAccountModificationRequestStatusOperation {
        AccountModificationRequest::Id m_id;
        Beam::Services::Tests::ServiceResult<AccountModificationRequest::Update>
          m_result;
      };

      /** Records a call to approve_account_modification_request(). */
      struct ApproveAccountModificationRequestOperation {
        AccountModificationRequest::Id m_id;
        Message m_comment;
        Beam::Services::Tests::ServiceResult<AccountModificationRequest::Update>
          m_result;
      };

      /** Records a call to reject_account_modification_request(). */
      struct RejectAccountModificationRequestOperation {
        AccountModificationRequest::Id m_id;
        Message m_comment;
        Beam::Services::Tests::ServiceResult<AccountModificationRequest::Update>
          m_result;
      };

      /** Records a call to load_message(). */
      struct LoadMessageOperation {
        Message::Id m_id;
        Beam::Services::Tests::ServiceResult<Message> m_result;
      };

      /** Records a call to load_message_ids(). */
      struct LoadMessageIdsOperation {
        AccountModificationRequest::Id m_id;
        Beam::Services::Tests::ServiceResult<std::vector<Message::Id>> m_result;
      };

      /** Records a call to send_account_modification_request_message(). */
      struct SendAccountModificationRequestMessageOperation {
        AccountModificationRequest::Id m_id;
        Message m_message;
        Beam::Services::Tests::ServiceResult<Message> m_result;
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
        LoadRiskStateOperation, StoreRiskStateOperation,
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
      explicit TestAdministrationClient(Beam::ScopedQueueWriter<
        std::shared_ptr<Operation>> operations) noexcept;

      ~TestAdministrationClient();

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        load_accounts_by_roles(AccountRoles roles);
      Beam::ServiceLocator::DirectoryEntry load_administrators_root_entry();
      Beam::ServiceLocator::DirectoryEntry load_services_root_entry();
      Beam::ServiceLocator::DirectoryEntry load_trading_groups_root_entry();
      bool check_administrator(
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles load_account_roles(
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountRoles load_account_roles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);
      Beam::ServiceLocator::DirectoryEntry load_parent_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& account);
      AccountIdentity load_identity(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store_identity(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);
      TradingGroup load_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& directory);
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        load_managed_trading_groups(
          const Beam::ServiceLocator::DirectoryEntry& account);
      std::vector<Beam::ServiceLocator::DirectoryEntry> load_administrators();
      std::vector<Beam::ServiceLocator::DirectoryEntry> load_services();
      MarketDataService::EntitlementDatabase load_entitlements();
      std::vector<Beam::ServiceLocator::DirectoryEntry> load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements);
      const Beam::Publisher<RiskService::RiskParameters>&
        get_risk_parameters_publisher(
          const Beam::ServiceLocator::DirectoryEntry& account);
      void store_risk_parameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& risk_parameters);
      const Beam::Publisher<RiskService::RiskState>& get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store_risk_state(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& risk_state);
      AccountModificationRequest load_account_modification_request(
        AccountModificationRequest::Id id);
      std::vector<AccountModificationRequest::Id>
        load_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      std::vector<AccountModificationRequest::Id>
        load_managed_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id start_id, int max_count);
      EntitlementModification load_entitlement_modification(
        AccountModificationRequest::Id id);
      AccountModificationRequest submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);
      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);
      AccountModificationRequest submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
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
      Beam::ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;
      Beam::SynchronizedUnorderedSet<Beam::Services::Tests::BaseServiceResult*>
        m_pending_results;
      Beam::IO::OpenState m_open_state;

      TestAdministrationClient(const TestAdministrationClient&) = delete;
      TestAdministrationClient& operator =(
        const TestAdministrationClient&) = delete;
      template<typename T, typename R, typename... Args>
      R append_result(Args&&... args);
  };

  inline TestAdministrationClient::TestAdministrationClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestAdministrationClient::~TestAdministrationClient() {
    close();
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      TestAdministrationClient::load_accounts_by_roles(AccountRoles roles) {
    return append_result<LoadAccountsByRolesOperation,
      std::vector<Beam::ServiceLocator::DirectoryEntry>>(std::move(roles));
  }

  inline Beam::ServiceLocator::DirectoryEntry
      TestAdministrationClient::load_administrators_root_entry() {
    return append_result<LoadAdministratorsRootEntryOperation,
      Beam::ServiceLocator::DirectoryEntry>();
  }

  inline Beam::ServiceLocator::DirectoryEntry
      TestAdministrationClient::load_services_root_entry() {
    return append_result<
      LoadServicesRootEntryOperation, Beam::ServiceLocator::DirectoryEntry>();
  }

  inline Beam::ServiceLocator::DirectoryEntry
      TestAdministrationClient::load_trading_groups_root_entry() {
    return append_result<LoadTradingGroupsRootEntryOperation,
      Beam::ServiceLocator::DirectoryEntry>();
  }

  inline bool TestAdministrationClient::check_administrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return append_result<CheckAdministratorOperation, bool>(account);
  }

  inline AccountRoles TestAdministrationClient::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return append_result<LoadAccountRolesOperation, AccountRoles>(account);
  }

  inline AccountRoles TestAdministrationClient::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return append_result<LoadParentChildAccountRolesOperation, AccountRoles>(
      parent, child);
  }

  inline Beam::ServiceLocator::DirectoryEntry
      TestAdministrationClient::load_parent_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return append_result<LoadParentTradingGroupOperation,
      Beam::ServiceLocator::DirectoryEntry>(account);
  }

  inline AccountIdentity TestAdministrationClient::load_identity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return append_result<LoadIdentityOperation, AccountIdentity>(account);
  }

  inline void TestAdministrationClient::store_identity(
      const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) {
    return append_result<StoreIdentityOperation, void>(account, identity);
  }

  inline TradingGroup TestAdministrationClient::load_trading_group(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    return append_result<LoadTradingGroupOperation, TradingGroup>(directory);
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      TestAdministrationClient::load_managed_trading_groups(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return append_result<LoadManagedTradingGroupsOperation,
      std::vector<Beam::ServiceLocator::DirectoryEntry>>(account);
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      TestAdministrationClient::load_administrators() {
    return append_result<LoadAdministratorsOperation,
      std::vector<Beam::ServiceLocator::DirectoryEntry>>();
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      TestAdministrationClient::load_services() {
    return append_result<LoadServicesOperation,
      std::vector<Beam::ServiceLocator::DirectoryEntry>>();
  }

  inline MarketDataService::EntitlementDatabase
      TestAdministrationClient::load_entitlements() {
    return append_result<LoadEntitlementsOperation,
      MarketDataService::EntitlementDatabase>();
  }

  inline std::vector<Beam::ServiceLocator::DirectoryEntry>
      TestAdministrationClient::load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return append_result<LoadAccountEntitlementsOperation,
      std::vector<Beam::ServiceLocator::DirectoryEntry>>(account);
  }

  inline void TestAdministrationClient::store_entitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    return append_result<StoreEntitlementsOperation, void>(
      account, entitlements);
  }

  inline const Beam::Publisher<RiskService::RiskParameters>&
      TestAdministrationClient::get_risk_parameters_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    throw std::runtime_error("Not supported.");
  }

  inline void TestAdministrationClient::store_risk_parameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& risk_parameters) {
    return append_result<StoreRiskParametersOperation, void>(
      account, risk_parameters);
  }

  inline const Beam::Publisher<RiskService::RiskState>&
      TestAdministrationClient::get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    throw std::runtime_error("Not supported.");
  }

  inline void TestAdministrationClient::store_risk_state(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& risk_state) {
    return append_result<StoreRiskStateOperation, void>(account, risk_state);
  }

  inline AccountModificationRequest
      TestAdministrationClient::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return append_result<
      LoadAccountModificationRequestOperation, AccountModificationRequest>(id);
  }

  inline std::vector<AccountModificationRequest::Id>
      TestAdministrationClient::load_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return append_result<LoadAccountModificationRequestIdsOperation,
      std::vector<AccountModificationRequest::Id>>(
        account, start_id, max_count);
  }

  inline std::vector<AccountModificationRequest::Id>
      TestAdministrationClient::load_managed_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id start_id, int max_count) {
    return append_result<LoadManagedAccountModificationRequestIdsOperation,
      std::vector<AccountModificationRequest::Id>>(
        account, start_id, max_count);
  }

  inline EntitlementModification
      TestAdministrationClient::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    return append_result<
      LoadEntitlementModificationOperation, EntitlementModification>(id);
  }

  inline AccountModificationRequest
      TestAdministrationClient::submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification,
        const Message& comment) {
    return append_result<SubmitEntitlementModificationRequestOperation,
      AccountModificationRequest>(account, modification, comment);
  }

  inline RiskModification TestAdministrationClient::load_risk_modification(
      AccountModificationRequest::Id id) {
    return append_result<LoadRiskModificationOperation, RiskModification>(id);
  }

  inline AccountModificationRequest
      TestAdministrationClient::submit_account_modification_request(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) {
    return append_result<SubmitRiskModificationRequestOperation,
      AccountModificationRequest>(account, modification, comment);
  }

  inline AccountModificationRequest::Update
      TestAdministrationClient::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return append_result<LoadAccountModificationRequestStatusOperation,
      AccountModificationRequest::Update>(id);
  }

  inline AccountModificationRequest::Update
      TestAdministrationClient::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return append_result<ApproveAccountModificationRequestOperation,
      AccountModificationRequest::Update>(id, comment);
  }

  inline AccountModificationRequest::Update
      TestAdministrationClient::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return append_result<RejectAccountModificationRequestOperation,
      AccountModificationRequest::Update>(id, comment);
  }

  inline Message TestAdministrationClient::load_message(Message::Id id) {
    return append_result<LoadMessageOperation, Message>(id);
  }

  inline std::vector<Message::Id>
      TestAdministrationClient::load_message_ids(
        AccountModificationRequest::Id id) {
    return append_result<LoadMessageIdsOperation, std::vector<Message::Id>>(id);
  }

  inline Message TestAdministrationClient::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    return append_result<
      SendAccountModificationRequestMessageOperation, Message>(id, message);
  }

  inline void TestAdministrationClient::close() {
    if(m_open_state.SetClosing()) {
      m_pending_results.With([&] (auto& results) {
        for(auto& result : results) {
          result->set(std::make_exception_ptr(Beam::IO::EndOfFileException()));
        }
      });
      m_pending_results.Clear();
    }
    m_open_state.Close();
  }

  template<typename T, typename R, typename... Args>
  R TestAdministrationClient::append_result(Args&&... args) {
    auto async = Beam::Routines::Async<R>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<T>, std::forward<Args>(args)..., async.GetEval());
    m_pending_results.Insert(&std::get<T>(*operation).m_result);
    if(!m_open_state.IsOpen()) {
      m_pending_results.Erase(&std::get<T>(*operation).m_result);
      BOOST_THROW_EXCEPTION(Beam::IO::EndOfFileException());
    }
    m_operations.Push(operation);
    auto result = std::move(async.Get());
    m_pending_results.Erase(&std::get<T>(*operation).m_result);
    if constexpr(!std::is_same_v<R, void>) {
      return result;
    }
  }
}

#endif
