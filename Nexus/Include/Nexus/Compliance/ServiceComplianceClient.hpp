#ifndef NEXUS_SERVICE_COMPLIANCE_CLIENT_HPP
#define NEXUS_SERVICE_COMPLIANCE_CLIENT_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Threading/CallOnce.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Compliance/ComplianceServices.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus::Compliance {

  /**
   * Client used to access the Compliance service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceComplianceClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a ComplianceClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename CF>
      explicit ServiceComplianceClient(CF&& clientBuilder);

      ~ServiceComplianceClient();
      std::vector<ComplianceRuleEntry> load(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry);
      ComplianceRuleEntry::Id add(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);
      void update(const ComplianceRuleEntry& entry);
      void remove(ComplianceRuleEntry::Id id);
      void report(const ComplianceRuleViolationRecord& record);
      void monitor_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);
      void close();

    private:
      struct PublisherEntry {
        Beam::Threading::CallOnce<Beam::Threading::Mutex> m_initializer;
        Beam::QueueWriterPublisher<ComplianceRuleEntry> m_publisher;
      };
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_client_handler;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<PublisherEntry>> m_publishers;
      Beam::IO::OpenState m_open_state;

      ServiceComplianceClient(const ServiceComplianceClient&) = delete;
      ServiceComplianceClient& operator =(
        const ServiceComplianceClient&) = delete;
      void on_compliance_rule_entry(
        ServiceProtocolClient& client, const ComplianceRuleEntry& entry);
  };

  template<typename B>
  template<typename CF>
  ServiceComplianceClient<B>::ServiceComplianceClient(CF&& client_builder)
      try : m_client_handler(std::forward<CF>(client_builder)) {
    RegisterComplianceServices(Store(m_client_handler.GetSlots()));
    RegisterComplianceMessages(Store(m_client_handler.GetSlots()));
    Beam::Services::AddMessageSlot<ComplianceRuleEntryMessage>(
      Store(m_client_handler.GetSlots()), std::bind_front(
        &ServiceComplianceClient::on_compliance_rule_entry, this));
  } catch(const std::exception&) {
    Beam::Services::RethrowNestedServiceException(
      "Failed to connect to the compliance server.");
  }

  template<typename B>
  ServiceComplianceClient<B>::~ServiceComplianceClient() {
    close();
  }

  template<typename B>
  std::vector<ComplianceRuleEntry> ServiceComplianceClient<B>::load(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<
        LoadDirectoryEntryComplianceRuleEntryService>(directory_entry);
    }, "Failed to load compliance rule entries: " +
      boost::lexical_cast<std::string>(directory_entry));
  }

  template<typename B>
  ComplianceRuleEntry::Id ServiceComplianceClient<B>::add(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      return client->template SendRequest<AddComplianceRuleEntryService>(
        directory_entry, state, schema);
    }, "Failed to add compliance rule entry: " +
      boost::lexical_cast<std::string>(directory_entry));
  }

  template<typename B>
  void ServiceComplianceClient<B>::update(const ComplianceRuleEntry& entry) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<UpdateComplianceRuleEntryService>(entry);
    }, "Failed to update compliance entry: " +
      boost::lexical_cast<std::string>(entry.get_id()));
  }

  template<typename B>
  void ServiceComplianceClient<B>::remove(ComplianceRuleEntry::Id id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      client->template SendRequest<DeleteComplianceRuleEntryService>(id);
    }, "Failed to delete compliance rule entry: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void ServiceComplianceClient<B>::report(
      const ComplianceRuleViolationRecord& record) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_client_handler.GetClient();
      Beam::Services::SendRecordMessage<ReportComplianceRuleViolationMessage>(
        *client, record);
    }, "Failed to report violation: " +
      boost::lexical_cast<std::string>(record.m_account) + ", " +
      boost::lexical_cast<std::string>(record.m_order_id));
  }

  template<typename B>
  void ServiceComplianceClient<B>::monitor_compliance_rule_entries(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto publisher = m_publishers.GetOrInsert(directory_entry,
        boost::factory<std::shared_ptr<PublisherEntry>>());
      auto client = m_client_handler.GetClient();
      publisher->m_initializer.Call([&] {
        client->template SendRequest<MonitorComplianceRuleEntryService>(
          directory_entry);
      });
      publisher->m_publisher.With([&] {
        *snapshot = client->template SendRequest<
          LoadDirectoryEntryComplianceRuleEntryService>(directory_entry);
        publisher->m_publisher.Monitor(std::move(queue));
      });
    }, "Failed to monitor compliance rule entries: " +
      boost::lexical_cast<std::string>(directory_entry));
  }

  template<typename B>
  void ServiceComplianceClient<B>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_client_handler.Close();
    m_publishers.Clear();
    m_open_state.Close();
  }

  template<typename B>
  void ServiceComplianceClient<B>::on_compliance_rule_entry(
      ServiceProtocolClient& client, const ComplianceRuleEntry& entry) {
    if(auto publisher = m_publishers.FindValue(entry.get_directory_entry())) {
      (*publisher)->m_publisher.Push(entry);
    }
  }
}

#endif
