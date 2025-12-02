#ifndef NEXUS_SERVICE_COMPLIANCE_CLIENT_HPP
#define NEXUS_SERVICE_COMPLIANCE_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/Async.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"
#include "Nexus/Compliance/ComplianceServices.hpp"

namespace Nexus {

  /**
   * Client used to access the Compliance service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceComplianceClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::dereference_t<B>;

      /**
       * Constructs a ComplianceClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<Beam::Initializes<B> CF>
      explicit ServiceComplianceClient(CF&& client_builder);

      ~ServiceComplianceClient();

      std::vector<ComplianceRuleEntry> load(
        const Beam::DirectoryEntry& directory_entry);
      ComplianceRuleEntry::Id add(
        const Beam::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);
      void update(const ComplianceRuleEntry& entry);
      void remove(ComplianceRuleEntry::Id id);
      void report(const ComplianceRuleViolationRecord& record);
      void monitor_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);
      void close();

    private:
      struct ComplianceRuleEntryMonitor {
        Beam::QueueWriterPublisher<ComplianceRuleEntry> m_publisher;
        std::vector<ComplianceRuleEntry> m_snapshot;
      };
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      std::unordered_map<Beam::DirectoryEntry,
        std::shared_ptr<ComplianceRuleEntryMonitor>> m_monitors;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      ServiceComplianceClient(const ServiceComplianceClient&) = delete;
      ServiceComplianceClient& operator =(
        const ServiceComplianceClient&) = delete;
      void recover_compliance_rule_entries(ServiceProtocolClient& client);
      void on_reconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void on_compliance_rule_entry(
        ServiceProtocolClient& client, const ComplianceRuleEntry& entry);
  };

  template<typename B>
  template<Beam::Initializes<B> CF>
  ServiceComplianceClient<B>::ServiceComplianceClient(CF&& client_builder)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_client_handler(std::forward<CF>(client_builder),
              std::bind_front(&ServiceComplianceClient::on_reconnect, this)) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    register_compliance_services(out(m_client_handler.get_slots()));
    register_compliance_messages(out(m_client_handler.get_slots()));
    Beam::add_message_slot<ComplianceRuleEntryMessage>(
      out(m_client_handler.get_slots()), std::bind_front(
        &ServiceComplianceClient::on_compliance_rule_entry, this));
  } catch(const std::exception&) {
    Beam::rethrow_nested_service_exception(
      "Failed to connect to the compliance server.");
  }

  template<typename B>
  ServiceComplianceClient<B>::~ServiceComplianceClient() {
    close();
  }

  template<typename B>
  std::vector<ComplianceRuleEntry> ServiceComplianceClient<B>::load(
      const Beam::DirectoryEntry& directory_entry) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<
        LoadDirectoryEntryComplianceRuleEntryService>(directory_entry);
    }, "Failed to load compliance rule entries: " +
      boost::lexical_cast<std::string>(directory_entry));
  }

  template<typename B>
  ComplianceRuleEntry::Id ServiceComplianceClient<B>::add(
      const Beam::DirectoryEntry& directory_entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      return client->template send_request<AddComplianceRuleEntryService>(
        directory_entry, state, schema);
    }, "Failed to add compliance rule entry: " +
      boost::lexical_cast<std::string>(directory_entry));
  }

  template<typename B>
  void ServiceComplianceClient<B>::update(const ComplianceRuleEntry& entry) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<UpdateComplianceRuleEntryService>(entry);
    }, "Failed to update compliance entry: " +
      boost::lexical_cast<std::string>(entry.get_id()));
  }

  template<typename B>
  void ServiceComplianceClient<B>::remove(ComplianceRuleEntry::Id id) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      client->template send_request<DeleteComplianceRuleEntryService>(id);
    }, "Failed to delete compliance rule entry: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void ServiceComplianceClient<B>::report(
      const ComplianceRuleViolationRecord& record) {
    return Beam::service_or_throw_with_nested([&] {
      auto client = m_client_handler.get_client();
      Beam::send_record_message<ReportComplianceRuleViolationMessage>(
        *client, record);
    }, "Failed to report violation: " +
      boost::lexical_cast<std::string>(record.m_account) + ", " +
      boost::lexical_cast<std::string>(record.m_order_id));
  }

  template<typename B>
  void ServiceComplianceClient<B>::monitor_compliance_rule_entries(
      const Beam::DirectoryEntry& directory_entry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    auto completion = Beam::Async<void>();
    m_tasks.push([&] {
      auto i = m_monitors.find(directory_entry);
      if(i == m_monitors.end()) {
        try {
          auto client = m_client_handler.get_client();
          auto snapshot =
            client->template send_request<MonitorComplianceRuleEntryService>(
              directory_entry);
          std::sort(snapshot.begin(), snapshot.end(),
            [] (const auto& lhs, const auto& rhs) {
              return lhs.get_id() < rhs.get_id();
            });
          auto monitor = std::make_shared<ComplianceRuleEntryMonitor>();
          monitor->m_snapshot = std::move(snapshot);
          i = m_monitors.insert(
            std::pair(directory_entry, std::move(monitor))).first;
        } catch(const std::exception&) {
          completion.get_eval().set_exception(
            Beam::make_nested_service_exception(
              "Failed to monitor compliance rule entries: " +
                boost::lexical_cast<std::string>(directory_entry)));
          return;
        }
      }
      auto monitor = i->second;
      monitor->m_publisher.monitor(std::move(queue));
      *snapshot = monitor->m_snapshot;
      completion.get_eval().set();
    });
    completion.get();
  }

  template<typename B>
  void ServiceComplianceClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_tasks.wait();
    m_client_handler.close();
    m_monitors.clear();
    m_open_state.close();
  }

  template<typename B>
  void ServiceComplianceClient<B>::recover_compliance_rule_entries(
      ServiceProtocolClient& client) {
    for(auto& [directory_entry, monitor] : m_monitors) {
      try {
        auto snapshot =
          client.template send_request<MonitorComplianceRuleEntryService>(
            directory_entry);
        std::sort(snapshot.begin(), snapshot.end(),
          [] (const auto& lhs, const auto& rhs) {
            return lhs.get_id() < rhs.get_id();
          });
        for(auto& entry : snapshot) {
          auto i = std::lower_bound(
            monitor->m_snapshot.begin(), monitor->m_snapshot.end(),
            entry.get_id(), [] (const auto& lhs, auto id) {
              return lhs.get_id() < id;
            });
          if(i == monitor->m_snapshot.end() ||
              i->get_id() != entry.get_id() || *i != entry) {
            monitor->m_publisher.push(entry);
          }
        }
        monitor->m_snapshot = std::move(snapshot);
      } catch(const std::exception&) {
        monitor->m_publisher.close(Beam::make_nested_service_exception(
          "Failed to monitor compliance rule entries: " +
            boost::lexical_cast<std::string>(directory_entry)));
      }
    }
  }

  template<typename B>
  void ServiceComplianceClient<B>::on_reconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.push([=, this] {
      recover_compliance_rule_entries(*client);
    });
  }

  template<typename B>
  void ServiceComplianceClient<B>::on_compliance_rule_entry(
      ServiceProtocolClient& client, const ComplianceRuleEntry& entry) {
    m_tasks.push([=, this] {
      auto i = m_monitors.find(entry.get_directory_entry());
      if(i == m_monitors.end()) {
        return;
      }
      auto monitor = i->second;
      auto j = std::lower_bound(
        monitor->m_snapshot.begin(), monitor->m_snapshot.end(), entry.get_id(),
        [] (const auto& lhs, auto id) {
          return lhs.get_id() < id;
        });
      if(j == monitor->m_snapshot.end() || j->get_id() != entry.get_id()) {
        monitor->m_snapshot.insert(j, entry);
      } else {
        *j = entry;
      }
      monitor->m_publisher.push(entry);
    });
  }
}

#endif
