#ifndef NEXUS_SERVICE_ADMINISTRATION_CLIENT_HPP
#define NEXUS_SERVICE_ADMINISTRATION_CLIENT_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/AdministrationService/AdministrationServices.hpp"

namespace Nexus {

  /**
   * Client used to access the Nexus Administration service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceAdministrationClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs an ServiceAdministrationClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceAdministrationClient(BF&& client_builder);

      ~ServiceAdministrationClient();

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
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
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
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& parameters);
      const Beam::Publisher<RiskService::RiskState>& get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account);
      void store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& state);
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
      AccountModificationRequest submit(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment);
      RiskModification load_risk_modification(
        AccountModificationRequest::Id id);
      AccountModificationRequest submit(
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
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using RiskParameterPublisher =
        Beam::StatePublisher<RiskService::RiskParameters>;
      using RiskStatePublisher = Beam::StatePublisher<RiskService::RiskState>;
      Beam::Services::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::IO::OpenState m_open_state;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskParameterPublisher>> m_risk_parameter_publishers;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskStatePublisher>> m_risk_state_publishers;
      Beam::RoutineTaskQueue m_tasks;

      ServiceAdministrationClient(const ServiceAdministrationClient&) = delete;
      ServiceAdministrationClient& operator =(
        const ServiceAdministrationClient&) = delete;
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void recover_risk_parameters(ServiceProtocolClient& client);
      void recover_risk_state(ServiceProtocolClient& client);
      void on_risk_parameters_message(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& risk_parameters);
      void on_risk_state_message(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        RiskService::RiskState risk_state);
  };

  template<typename B>
  template<typename BF>
  ServiceAdministrationClient<B>::ServiceAdministrationClient(
    BF&& client_builder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_client_handler(std::forward<BF>(client_builder), std::bind_front(
              &ServiceAdministrationClient::on_reconnect, this)) {
    RegisterAdministrationServices(Beam::Store(m_client_handler.GetSlots()));
    RegisterAdministrationMessages(Beam::Store(m_client_handler.GetSlots()));
    Beam::Services::AddMessageSlot<RiskParametersMessage>(
      Beam::Store(m_client_handler.GetSlots()), std::bind_front(
        &ServiceAdministrationClient::on_risk_parameters_message, this));
    Beam::Services::AddMessageSlot<RiskStateMessage>(
      Beam::Store(m_client_handler.GetSlots()), std::bind_front(
        &ServiceAdministrationClient::on_risk_state_message, this));
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::IO::ConnectException(
      "Failed to connect to the administration server."));
  }

  template<typename B>
  ServiceAdministrationClient<B>::~ServiceAdministrationClient() {
    close();
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ServiceAdministrationClient<B>::load_accounts_by_roles(
        AccountRoles roles) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadAccountsByRolesService>(roles);
    }, "Failed to load accounts by roles: " +
      boost::lexical_cast<std::string>(roles));
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      ServiceAdministrationClient<B>::load_administrators_root_entry() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadAdministratorsRootEntryService>();
    }, "Failed to load administrators root entry.");
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      ServiceAdministrationClient<B>::load_services_root_entry() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadServicesRootEntryService>();
    }, "Failed to load services root entry.");
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      ServiceAdministrationClient<B>::load_trading_groups_root_entry() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadTradingGroupsRootEntryService>();
    }, "Failed to load trading groups root entry.");
  }

  template<typename B>
  bool ServiceAdministrationClient<B>::check_administrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<CheckAdministratorService>(account);
    }, "Failed to check administrator: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountRoles ServiceAdministrationClient<B>::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadAccountRolesService>(account);
    }, "Failed to load account roles: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountRoles ServiceAdministrationClient<B>::load_account_roles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadSupervisedAccountRolesService>(
        parent, child);
    }, "Failed to load account roles: " +
      boost::lexical_cast<std::string>(parent) + ", " +
      boost::lexical_cast<std::string>(child));
  }

  template<typename B>
  Beam::ServiceLocator::DirectoryEntry
      ServiceAdministrationClient<B>::load_parent_trading_group(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadParentTradingGroupService>(
        account);
    }, "Failed to load parent trading group: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountIdentity ServiceAdministrationClient<B>::load_identity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadAccountIdentityService>(account);
    }, "Failed to load identity: " + boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ServiceAdministrationClient<B>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<StoreAccountIdentityService>(
        account, identity);
    }, "Failed to store identity: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  TradingGroup ServiceAdministrationClient<B>::load_trading_group(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadTradingGroupService>(directory);
    }, "Failed to load trading group: " +
      boost::lexical_cast<std::string>(directory));
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ServiceAdministrationClient<B>::load_managed_trading_groups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadManagedTradingGroupsService>(
        account);
    }, "Failed to load managed trading groups: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ServiceAdministrationClient<B>::load_administrators() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadAdministratorsService>();
    }, "Failed to load administrators.");
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ServiceAdministrationClient<B>::load_services() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadServicesService>();
    }, "Failed to load services.");
  }

  template<typename B>
  MarketDataService::EntitlementDatabase
      ServiceAdministrationClient<B>::load_entitlements() {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadEntitlementsService>();
    }, "Failed to load entitlements.");
  }

  template<typename B>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ServiceAdministrationClient<B>::load_entitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadAccountEntitlementsService>(
        account);
    }, "Failed to load entitlements: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ServiceAdministrationClient<B>::store_entitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<StoreEntitlementsService>(
        account, entitlements);
    }, "Failed to store entitlements: " +
      boost::lexical_cast<std::string>(account) +
      boost::lexical_cast<std::string>(Beam::Stream(entitlements)));
  }

  template<typename B>
  const Beam::Publisher<RiskService::RiskParameters>&
      ServiceAdministrationClient<B>::get_risk_parameters_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_risk_parameter_publishers.GetOrInsert(account, [&] {
      auto publisher = std::make_shared<RiskParameterPublisher>();
      m_tasks.Push([=, this] {
        try {
          auto client = m_client_handler.GetClient();
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
  void ServiceAdministrationClient<B>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& parameters) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<StoreRiskParametersService>(
        account, parameters);
    }, "Failed to store risk parameters: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(parameters));
  }

  template<typename B>
  const Beam::Publisher<RiskService::RiskState>&
      ServiceAdministrationClient<B>::get_risk_state_publisher(
        const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_risk_state_publishers.GetOrInsert(account, [&] {
      auto publisher = std::make_shared<RiskStatePublisher>();
      m_tasks.Push([=, this] {
        try {
          auto client = m_client_handler.GetClient();
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
  void ServiceAdministrationClient<B>::store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& state) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<StoreRiskStateService>(account, state);
    }, "Failed to store risk state: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(state));
  }

  template<typename B>
  AccountModificationRequest
      ServiceAdministrationClient<B>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        LoadAccountModificationRequestService>(id);
    }, "Failed to load account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  std::vector<AccountModificationRequest::Id>
      ServiceAdministrationClient<B>::load_account_modification_request_ids(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id id, int max_count) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        LoadAccountModificationRequestIdsService>(account, id, max_count);
    }, "Failed to load account modification request ids: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(id) + ", " +
      boost::lexical_cast<std::string>(max_count));
  }

  template<typename B>
  std::vector<AccountModificationRequest::Id>
      ServiceAdministrationClient<B>::
        load_managed_account_modification_request_ids(
          const Beam::ServiceLocator::DirectoryEntry& account,
          AccountModificationRequest::Id id, int max_count) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        LoadManagedAccountModificationRequestIdsService>(
          account, id, max_count);
    }, "Failed to load managed account modification request ids: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(id) + ", " +
      boost::lexical_cast<std::string>(max_count));
  }

  template<typename B>
  EntitlementModification
      ServiceAdministrationClient<B>::load_entitlement_modification(
        AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadEntitlementModificationService>(
        id);
    }, "Failed to load entitlement modification: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest
      ServiceAdministrationClient<B>::submit(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        SubmitEntitlementModificationRequestService>(
          account, modification, comment);
    }, "Failed to submit account modification request: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  RiskModification ServiceAdministrationClient<B>::load_risk_modification(
      AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadRiskModificationService>(id);
    }, "Failed to load risk modification: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest
      ServiceAdministrationClient<B>::submit(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<SubmitRiskModificationRequestService>(
        account, modification, comment);
    }, "Failed to submit account modification request: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountModificationRequest::Update
      ServiceAdministrationClient<B>::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        LoadAccountModificationRequestStatusService>(id);
    }, "Failed to load account modification request status: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest::Update
      ServiceAdministrationClient<B>::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        ApproveAccountModificationRequestService>(id, comment);
    }, "Failed to approve account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest::Update
      ServiceAdministrationClient<B>::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        RejectAccountModificationRequestService>(id, comment);
    }, "Failed to reject account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  Message ServiceAdministrationClient<B>::load_message(Message::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadMessageService>(id);
    }, "Failed to load message: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  std::vector<Message::Id> ServiceAdministrationClient<B>::load_message_ids(
      AccountModificationRequest::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<LoadMessageIdsService>(id);
    }, "Failed to load message ids: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  Message ServiceAdministrationClient<B>::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        SendAccountModificationRequestMessageService>(id, message);
    }, "Failed to send account modification request message: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void ServiceAdministrationClient<B>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_client_handler.Close();
    m_risk_state_publishers.Clear();
    m_risk_parameter_publishers.Clear();
    m_open_state.Close();
  }

  template<typename B>
  void ServiceAdministrationClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.Push([=, this] {
      recover_risk_parameters(*client);
      recover_risk_state(*client);
    });
  }

  template<typename B>
  void ServiceAdministrationClient<B>::recover_risk_parameters(
      ServiceProtocolClient& client) {
    auto current_risk_parameter_entries =
      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskParameterPublisher>>>();
    m_risk_parameter_publishers.With([&] (auto& risk_parameter_publishers) {
      for(auto& publisher : risk_parameter_publishers) {
        current_risk_parameter_entries.emplace_back(
          publisher.first, publisher.second);
      }
    });
    for(auto& entry : current_risk_parameter_entries) {
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
  void ServiceAdministrationClient<B>::recover_risk_state(
      ServiceProtocolClient& client) {
    auto current_risk_state_entries =
      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskStatePublisher>>>();
    m_risk_state_publishers.With([&] (auto& riskStatePublishers) {
      for(auto& publisher : riskStatePublishers) {
        current_risk_state_entries.emplace_back(
          publisher.first, publisher.second);
      }
    });
    for(auto& entry : current_risk_state_entries) {
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
  void ServiceAdministrationClient<B>::on_risk_parameters_message(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& risk_parameters) {
    m_tasks.Push([=, this] {
      if(auto publisher = m_risk_parameter_publishers.FindValue(account)) {
        try {
          (*publisher)->Push(risk_parameters);
        } catch(const Beam::PipeBrokenException&) {
          m_risk_parameter_publishers.Erase(account);
        }
      }
    });
  }

  template<typename B>
  void ServiceAdministrationClient<B>::on_risk_state_message(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      RiskService::RiskState risk_state) {
    m_tasks.Push([=, this] {
      if(auto publisher = m_risk_state_publishers.FindValue(account)) {
        try {
          (*publisher)->Push(risk_state);
        } catch(const Beam::PipeBrokenException&) {
          m_risk_state_publishers.Erase(account);
        }
      }
    });
  }
}

#endif
