#ifndef NEXUS_COMPLIANCE_CLIENT_HPP
#define NEXUS_COMPLIANCE_CLIENT_HPP
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
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Compliance/ComplianceServices.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus::Compliance {

  /**
   * Client used to access the Compliance service.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ComplianceClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<B>;

      /**
       * Constructs a ComplianceClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename CF>
      explicit ComplianceClient(CF&& clientBuilder);

      ~ComplianceClient();

      std::vector<ComplianceRuleEntry> Load(
        const Beam::ServiceLocator::DirectoryEntry& entry);

      ComplianceRuleId Add(const Beam::ServiceLocator::DirectoryEntry& entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);

      void Update(const ComplianceRuleEntry& entry);

      void Delete(ComplianceRuleId id);

      void Report(const ComplianceRuleViolationRecord& violationRecord);

      void MonitorComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);

      void Close();

    private:
      struct PublisherEntry {
        Beam::Threading::CallOnce<Beam::Threading::Mutex> m_initializer;
        Beam::QueueWriterPublisher<ComplianceRuleEntry> m_publisher;
      };
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<PublisherEntry>> m_publishers;
      Beam::IO::OpenState m_openState;

      ComplianceClient(const ComplianceClient&) = delete;
      ComplianceClient& operator =(const ComplianceClient&) = delete;
      void OnComplianceRuleEntry(ServiceProtocolClient& client,
        const ComplianceRuleEntry& entry);
  };

  template<typename B>
  template<typename CF>
  ComplianceClient<B>::ComplianceClient(CF&& clientBuilder)
      try : m_clientHandler(std::forward<CF>(clientBuilder)) {
    RegisterComplianceServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterComplianceMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<ComplianceRuleEntryMessage>(
      Store(m_clientHandler.GetSlots()),
      std::bind_front(&ComplianceClient::OnComplianceRuleEntry, this));
  } catch(const std::exception&) {
    Beam::Services::RethrowNestedServiceException(
      "Failed to connect to the compliance server.");
  }

  template<typename B>
  ComplianceClient<B>::~ComplianceClient() {
    Close();
  }

  template<typename B>
  std::vector<ComplianceRuleEntry> ComplianceClient<B>::Load(
      const Beam::ServiceLocator::DirectoryEntry& entry) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<
        LoadDirectoryEntryComplianceRuleEntryService>(entry);
    }, "Failed to load compliance rule entries: " +
      boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  ComplianceRuleId ComplianceClient<B>::Add(
      const Beam::ServiceLocator::DirectoryEntry& entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      return client->template SendRequest<AddComplianceRuleEntryService>(entry,
        state, schema);
    }, "Failed to add compliance rule entry: " +
      boost::lexical_cast<std::string>(entry));
  }

  template<typename B>
  void ComplianceClient<B>::Update(const ComplianceRuleEntry& entry) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<UpdateComplianceRuleEntryService>(entry);
    }, "Failed to update compliance entry: " +
      boost::lexical_cast<std::string>(entry.GetId()));
  }

  template<typename B>
  void ComplianceClient<B>::Delete(ComplianceRuleId id) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      client->template SendRequest<DeleteComplianceRuleEntryService>(id);
    }, "Failed to delete compliance rule entry: " +
      boost::lexical_cast<std::string>(id));
  }

  template<typename B>
  void ComplianceClient<B>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto client = m_clientHandler.GetClient();
      Beam::Services::SendRecordMessage<ReportComplianceRuleViolationMessage>(
        *client, violationRecord);
    }, "Failed to report violation: " + boost::lexical_cast<std::string>(
      violationRecord.m_account) + ", " + boost::lexical_cast<std::string>(
      violationRecord.m_orderId));
  }

  template<typename B>
  void ComplianceClient<B>::MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return Beam::Services::ServiceOrThrowWithNested([&] {
      auto publisher = m_publishers.GetOrInsert(directoryEntry,
        boost::factory<std::shared_ptr<PublisherEntry>>());
      auto client = m_clientHandler.GetClient();
      publisher->m_initializer.Call([&] {
        client->template SendRequest<MonitorComplianceRuleEntryService>(
          directoryEntry);
      });
      publisher->m_publisher.With([&] {
        *snapshot = client->template SendRequest<
          LoadDirectoryEntryComplianceRuleEntryService>(directoryEntry);
        publisher->m_publisher.Monitor(std::move(queue));
      });
    }, "Failed to monitor compliance rule entries: " +
      boost::lexical_cast<std::string>(directoryEntry));
  }

  template<typename B>
  void ComplianceClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_publishers.Clear();
    m_openState.Close();
  }

  template<typename B>
  void ComplianceClient<B>::OnComplianceRuleEntry(ServiceProtocolClient& client,
      const ComplianceRuleEntry& entry) {
    if(auto publisher = m_publishers.FindValue(entry.GetDirectoryEntry())) {
      (*publisher)->m_publisher.Push(entry);
    }
  }
}

#endif
