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
      using ServiceProtocolClientBuilder = Beam::dereference_t<B>;

      /**
       * Constructs an ServiceAdministrationClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit ServiceAdministrationClient(BF&& client_builder);

      ~ServiceAdministrationClient();

      std::vector<Beam::DirectoryEntry>
        load_accounts_by_roles(AccountRoles roles);
      Beam::DirectoryEntry load_administrators_root_entry();
      Beam::DirectoryEntry load_services_root_entry();
      Beam::DirectoryEntry load_trading_groups_root_entry();
      bool check_administrator(const Beam::DirectoryEntry& account);
      AccountRoles load_account_roles(const Beam::DirectoryEntry& account);
      AccountRoles load_account_roles(
        const Beam::DirectoryEntry& parent, const Beam::DirectoryEntry& child);
      Beam::DirectoryEntry load_parent_trading_group(
        const Beam::DirectoryEntry& account);
      AccountIdentity load_identity(const Beam::DirectoryEntry& account);
      void store(
        const Beam::DirectoryEntry& account, const AccountIdentity& identity);
      TradingGroup load_trading_group(const Beam::DirectoryEntry& directory);
      std::vector<Beam::DirectoryEntry>
        load_managed_trading_groups(const Beam::DirectoryEntry& account);
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
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using RiskParameterPublisher = Beam::StatePublisher<RiskParameters>;
      using RiskStatePublisher = Beam::StatePublisher<RiskState>;
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::OpenState m_open_state;
      Beam::SynchronizedUnorderedMap<Beam::DirectoryEntry,
        std::shared_ptr<RiskParameterPublisher>> m_risk_parameter_publishers;
      Beam::SynchronizedUnorderedMap<Beam::DirectoryEntry,
        std::shared_ptr<RiskStatePublisher>> m_risk_state_publishers;
      Beam::RoutineTaskQueue m_tasks;

      ServiceAdministrationClient(const ServiceAdministrationClient&) = delete;
      ServiceAdministrationClient& operator =(
        const ServiceAdministrationClient&) = delete;
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void recover_risk_parameters(ServiceProtocolClient& client);
      void recover_risk_state(ServiceProtocolClient& client);
      void on_risk_parameters_message(
        ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
        const RiskParameters& risk_parameters);
      void on_risk_state_message(ServiceProtocolClient& client,
        const Beam::DirectoryEntry& account, RiskState risk_state);
  };

  template<typename B>
  template<typename BF>
  ServiceAdministrationClient<B>::ServiceAdministrationClient(
    BF&& client_builder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_client_handler(std::forward<BF>(client_builder), std::bind_front(
              &ServiceAdministrationClient::on_reconnect, this)) {
    register_administration_services(Beam::out(m_client_handler.get_slots()));
    register_administration_messages(Beam::out(m_client_handler.get_slots()));
    Beam::add_message_slot<RiskParametersMessage>(
      Beam::out(m_client_handler.get_slots()), std::bind_front(
        &ServiceAdministrationClient::on_risk_parameters_message, this));
    Beam::add_message_slot<RiskStateMessage>(
      Beam::out(m_client_handler.get_slots()), std::bind_front(
        &ServiceAdministrationClient::on_risk_state_message, this));
BEAM_UNSUPPRESS_THIS_INITIALIZER()
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to connect to the administration server."));
  }

  template<typename B>
  ServiceAdministrationClient<B>::~ServiceAdministrationClient() {
    close();
  }

  template<typename B>
  std::vector<Beam::DirectoryEntry>
      ServiceAdministrationClient<B>::load_accounts_by_roles(
        AccountRoles roles) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadAccountsByRolesService>(roles);
    }, "Failed to load accounts by roles: " +
      boost::lexical_cast<std::string>(roles));
  }

  template<typename B>
  Beam::DirectoryEntry
      ServiceAdministrationClient<B>::load_administrators_root_entry() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        LoadAdministratorsRootEntryService>();
    }, "Failed to load administrators root entry.");
  }

  template<typename B>
  Beam::DirectoryEntry
      ServiceAdministrationClient<B>::load_services_root_entry() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadServicesRootEntryService>();
    }, "Failed to load services root entry.");
  }

  template<typename B>
  Beam::DirectoryEntry
      ServiceAdministrationClient<B>::load_trading_groups_root_entry() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadTradingGroupsRootEntryService>();
    }, "Failed to load trading groups root entry.");
  }

  template<typename B>
  bool ServiceAdministrationClient<B>::check_administrator(
      const Beam::DirectoryEntry& account) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<CheckAdministratorService>(account);
    }, "Failed to check administrator: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountRoles ServiceAdministrationClient<B>::load_account_roles(
      const Beam::DirectoryEntry& account) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadAccountRolesService>(account);
    }, "Failed to load account roles: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountRoles ServiceAdministrationClient<B>::load_account_roles(
      const Beam::DirectoryEntry& parent,
      const Beam::DirectoryEntry& child) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadSupervisedAccountRolesService>(
        parent, child);
    }, "Failed to load account roles: " +
      boost::lexical_cast<std::string>(parent) + ", " +
      boost::lexical_cast<std::string>(child));
  }

  template<typename B>
  Beam::DirectoryEntry
      ServiceAdministrationClient<B>::load_parent_trading_group(
        const Beam::DirectoryEntry& account) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadParentTradingGroupService>(
        account);
    }, "Failed to load parent trading group: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountIdentity ServiceAdministrationClient<B>::load_identity(
      const Beam::DirectoryEntry& account) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadAccountIdentityService>(account);
    }, "Failed to load identity: " + boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ServiceAdministrationClient<B>::store(
      const Beam::DirectoryEntry& account,
      const AccountIdentity& identity) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<StoreAccountIdentityService>(
        account, identity);
    }, "Failed to store identity: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  TradingGroup ServiceAdministrationClient<B>::load_trading_group(
      const Beam::DirectoryEntry& directory) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadTradingGroupService>(directory);
    }, "Failed to load trading group: " +
      boost::lexical_cast<std::string>(directory));
  }

  template<typename B>
  std::vector<Beam::DirectoryEntry>
      ServiceAdministrationClient<B>::load_managed_trading_groups(
      const Beam::DirectoryEntry& account) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadManagedTradingGroupsService>(
        account);
    }, "Failed to load managed trading groups: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  std::vector<Beam::DirectoryEntry>
      ServiceAdministrationClient<B>::load_administrators() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadAdministratorsService>();
    }, "Failed to load administrators.");
  }

  template<typename B>
  std::vector<Beam::DirectoryEntry>
      ServiceAdministrationClient<B>::load_services() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadServicesService>();
    }, "Failed to load services.");
  }

  template<typename B>
  EntitlementDatabase ServiceAdministrationClient<B>::load_entitlements() {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadEntitlementsService>();
    }, "Failed to load entitlements.");
  }

  template<typename B>
  std::vector<Beam::DirectoryEntry>
      ServiceAdministrationClient<B>::load_entitlements(
        const Beam::DirectoryEntry& account) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadAccountEntitlementsService>(
        account);
    }, "Failed to load entitlements: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  void ServiceAdministrationClient<B>::store_entitlements(
      const Beam::DirectoryEntry& account,
      const std::vector<Beam::DirectoryEntry>& entitlements) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<StoreEntitlementsService>(
        account, entitlements);
    }, "Failed to store entitlements: " +
      boost::lexical_cast<std::string>(account) +
      boost::lexical_cast<std::string>(Beam::Stream(entitlements)));
  }

  template<typename B>
  const Beam::Publisher<RiskParameters>&
      ServiceAdministrationClient<B>::get_risk_parameters_publisher(
        const Beam::DirectoryEntry& account) {
    return *m_risk_parameter_publishers.get_or_insert(account, [&] {
      auto publisher = std::make_shared<RiskParameterPublisher>();
      m_tasks.push([=, this] {
        try {
          auto client = m_client_handler.get_client();
          auto parameters =
            client->template send_request<MonitorRiskParametersService>(
              account);
          publisher->push(parameters);
        } catch(const std::exception&) {
          publisher->close(Beam::make_nested_service_exception(
            "Failed to monitor risk parameters: " +
            boost::lexical_cast<std::string>(account)));
        }
      });
      return publisher;
    });
  }

  template<typename B>
  void ServiceAdministrationClient<B>::store(
      const Beam::DirectoryEntry& account, const RiskParameters& parameters) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<StoreRiskParametersService>(
        account, parameters);
    }, "Failed to store risk parameters: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(parameters));
  }

  template<typename B>
  const Beam::Publisher<RiskState>&
      ServiceAdministrationClient<B>::get_risk_state_publisher(
        const Beam::DirectoryEntry& account) {
    return *m_risk_state_publishers.get_or_insert(account, [&] {
      auto publisher = std::make_shared<RiskStatePublisher>();
      m_tasks.push([=, this] {
        try {
          auto client = m_client_handler.get_client();
          auto state = client->template send_request<MonitorRiskStateService>(
            account);
          publisher->push(state);
        } catch(const std::exception&) {
          publisher->close(Beam::make_nested_service_exception(
            "Failed to monitor risk state: " + boost::lexical_cast<std::string>(
              account)));
        }
      });
      return publisher;
    });
  }

  template<typename B>
  void ServiceAdministrationClient<B>::store(
      const Beam::DirectoryEntry& account, const RiskState& state) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<StoreRiskStateService>(account, state);
    }, "Failed to store risk state: " +
      boost::lexical_cast<std::string>(account) + ", " +
      boost::lexical_cast<std::string>(state));
  }

  template<typename B>
  AccountModificationRequest
      ServiceAdministrationClient<B>::load_account_modification_request(
        AccountModificationRequest::Id id) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        LoadAccountModificationRequestService>(id);
    }, "Failed to load account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  std::vector<AccountModificationRequest::Id>
      ServiceAdministrationClient<B>::load_account_modification_request_ids(
        const Beam::DirectoryEntry& account,
        AccountModificationRequest::Id id, int max_count) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
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
          const Beam::DirectoryEntry& account,
          AccountModificationRequest::Id id, int max_count) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
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
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadEntitlementModificationService>(
        id);
    }, "Failed to load entitlement modification: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest
      ServiceAdministrationClient<B>::submit(
        const Beam::DirectoryEntry& account,
        const EntitlementModification& modification, const Message& comment) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        SubmitEntitlementModificationRequestService>(
          account, modification, comment);
    }, "Failed to submit account modification request: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  RiskModification ServiceAdministrationClient<B>::load_risk_modification(
      AccountModificationRequest::Id id) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadRiskModificationService>(id);
    }, "Failed to load risk modification: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest
      ServiceAdministrationClient<B>::submit(
        const Beam::DirectoryEntry& account,
        const RiskModification& modification, const Message& comment) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        SubmitRiskModificationRequestService>(account, modification, comment);
    }, "Failed to submit account modification request: " +
      boost::lexical_cast<std::string>(account));
  }

  template<typename B>
  AccountModificationRequest::Update
      ServiceAdministrationClient<B>::load_account_modification_request_status(
        AccountModificationRequest::Id id) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        LoadAccountModificationRequestStatusService>(id);
    }, "Failed to load account modification request status: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest::Update
      ServiceAdministrationClient<B>::approve_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        ApproveAccountModificationRequestService>(id, comment);
    }, "Failed to approve account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  AccountModificationRequest::Update
      ServiceAdministrationClient<B>::reject_account_modification_request(
        AccountModificationRequest::Id id, const Message& comment) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        RejectAccountModificationRequestService>(id, comment);
    }, "Failed to reject account modification request: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  Message ServiceAdministrationClient<B>::load_message(Message::Id id) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadMessageService>(id);
    }, "Failed to load message: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  std::vector<Message::Id> ServiceAdministrationClient<B>::load_message_ids(
      AccountModificationRequest::Id id) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<LoadMessageIdsService>(id);
    }, "Failed to load message ids: " + boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  Message ServiceAdministrationClient<B>::
      send_account_modification_request_message(
        AccountModificationRequest::Id id, const Message& message) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        SendAccountModificationRequestMessageService>(id, message);
    }, "Failed to send account modification request message: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void ServiceAdministrationClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_tasks.wait();
    m_client_handler.close();
    m_risk_state_publishers.clear();
    m_risk_parameter_publishers.clear();
    m_open_state.close();
  }

  template<typename B>
  void ServiceAdministrationClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.push([=, this] {
      recover_risk_parameters(*client);
      recover_risk_state(*client);
    });
  }

  template<typename B>
  void ServiceAdministrationClient<B>::recover_risk_parameters(
      ServiceProtocolClient& client) {
    auto current_risk_parameter_entries =
      std::vector<std::tuple<Beam::DirectoryEntry,
        std::shared_ptr<RiskParameterPublisher>>>();
    m_risk_parameter_publishers.for_each([&] (auto& publisher) {
      current_risk_parameter_entries.emplace_back(
        publisher.first, publisher.second);
    });
    for(auto& entry : current_risk_parameter_entries) {
      auto& publisher = std::get<1>(entry);
      try {
        auto parameters =
          client.template send_request<MonitorRiskParametersService>(
            std::get<0>(entry));
        if(publisher->get_snapshot() != parameters) {
          publisher->push(parameters);
        }
      } catch(const std::exception&) {
        publisher->close(Beam::make_nested_service_exception(
          "Failed to recover risk parameters."));
      }
    }
  }

  template<typename B>
  void ServiceAdministrationClient<B>::recover_risk_state(
      ServiceProtocolClient& client) {
    auto current_risk_state_entries =
      std::vector<std::tuple<Beam::DirectoryEntry,
        std::shared_ptr<RiskStatePublisher>>>();
    m_risk_state_publishers.with([&] (auto& risk_state_publishers) {
      for(auto& publisher : risk_state_publishers) {
        current_risk_state_entries.emplace_back(
          publisher.first, publisher.second);
      }
    });
    for(auto& entry : current_risk_state_entries) {
      auto& publisher = std::get<1>(entry);
      try {
        auto state = client.template send_request<MonitorRiskStateService>(
          std::get<0>(entry));
        if(publisher->get_snapshot() != state) {
          publisher->push(state);
        }
      } catch(const std::exception&) {
        publisher->close(Beam::make_nested_service_exception(
          "Failed to recover risk state."));
      }
    }
  }

  template<typename B>
  void ServiceAdministrationClient<B>::on_risk_parameters_message(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
      const RiskParameters& risk_parameters) {
    m_tasks.push([=, this] {
      if(auto publisher = m_risk_parameter_publishers.try_load(account)) {
        try {
          (*publisher)->push(risk_parameters);
        } catch(const Beam::PipeBrokenException&) {
          m_risk_parameter_publishers.erase(account);
        }
      }
    });
  }

  template<typename B>
  void ServiceAdministrationClient<B>::on_risk_state_message(
      ServiceProtocolClient& client, const Beam::DirectoryEntry& account,
      RiskState risk_state) {
    m_tasks.push([=, this] {
      if(auto publisher = m_risk_state_publishers.try_load(account)) {
        try {
          (*publisher)->push(risk_state);
        } catch(const Beam::PipeBrokenException&) {
          m_risk_state_publishers.erase(account);
        }
      }
    });
  }
}

#endif
