#ifndef NEXUS_COMPLIANCE_CLIENT_HPP
#define NEXUS_COMPLIANCE_CLIENT_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Threading/CallOnce.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/functional/factory.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Compliance/Compliance.hpp"
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

      /**
       * Loads all ComplianceRuleEntries for a specified DirectoryEntry.
       * @param directoryEntry The DirectoryEntry to query.
       * @return The list of all ComplianceRuleEntries assigned to the
       *         <i>directoryEntry</i>.
       */
      std::vector<ComplianceRuleEntry> Load(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

      /**
       * Assigns a new compliance rule to a DirectoryEntry.
       * @param directoryEntry The DirectoryEntry to assign the rule to.
       * @param state The rule's initial State.
       * @param schema The ComplianceRuleSchema specifying the rule to add.
       * @return The id of the new entry.
       */
      ComplianceRuleId Add(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);

      /**
       * Updates an existing compliance rule.
       * @param entry The ComplianceRuleEntry to update.
       */
      void Update(const ComplianceRuleEntry& entry);

      /**
       * Deletes a ComplianceRuleEntry.
       * \param id The ComplianceRuleId to delete.
       */
      void Delete(ComplianceRuleId id);

      /**
       * Reports a compliance violation.
       * @param violationRecord The violation to report.
       */
      void Report(const ComplianceRuleViolationRecord& violationRecord);

      /**
       * Monitors updates to a DirectoryEntry's ComplianceRuleEntries.
       * @param directoryEntry The DirectoryEntry to monitor.
       * @param queue Stores the changes to the <i>directoryEntry</i>'s
       *        ComplianceRuleEntries.
       * @param snapshot Stores the snapshot of the current
       *        ComplianceRuleEntries.
       */
      void MonitorComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);

      void Close();

    private:
      struct ComplianceQueueEntry {
        Beam::Threading::Mutex m_mutex;
        std::vector<Beam::ScopedQueueWriter<ComplianceRuleEntry>> m_queues;
        Beam::Threading::CallOnce<Beam::Threading::Mutex> m_initializer;
      };
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<ComplianceQueueEntry>> m_complianceEntryQueues;
      Beam::IO::OpenState m_openState;

      ComplianceClient(const ComplianceClient&) = delete;
      ComplianceClient& operator =(const ComplianceClient&) = delete;
      void OnComplianceRuleEntry(ServiceProtocolClient& client,
        const ComplianceRuleEntry& entry);
  };

  template<typename B>
  template<typename CF>
  ComplianceClient<B>::ComplianceClient(CF&& clientBuilder)
      : m_clientHandler(std::forward<CF>(clientBuilder)) {
    RegisterComplianceServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterComplianceMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<ComplianceRuleEntryMessage>(
      Store(m_clientHandler.GetSlots()),
      std::bind(&ComplianceClient::OnComplianceRuleEntry, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename B>
  ComplianceClient<B>::~ComplianceClient() {
    Close();
  }

  template<typename B>
  std::vector<ComplianceRuleEntry> ComplianceClient<B>::Load(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadDirectoryEntryComplianceRuleEntryService>(directoryEntry);
  }

  template<typename B>
  ComplianceRuleId ComplianceClient<B>::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<AddComplianceRuleEntryService>(
      directoryEntry, state, schema);
  }

  template<typename B>
  void ComplianceClient<B>::Update(const ComplianceRuleEntry& entry) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<UpdateComplianceRuleEntryService>(entry);
  }

  template<typename B>
  void ComplianceClient<B>::Delete(ComplianceRuleId id) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<DeleteComplianceRuleEntryService>(id);
  }

  template<typename B>
  void ComplianceClient<B>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    auto client = m_clientHandler.GetClient();
    Beam::Services::SendRecordMessage<ReportComplianceRuleViolationMessage>(
      *client, violationRecord);
  }

  template<typename B>
  void ComplianceClient<B>::MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    auto entry = m_complianceEntryQueues.GetOrInsert(directoryEntry,
      boost::factory<std::shared_ptr<ComplianceQueueEntry>>());
    auto client = m_clientHandler.GetClient();
    entry->m_initializer.Call(
      [&] {
        client->template SendRequest<MonitorComplianceRuleEntryService>(
          directoryEntry);
      });
    auto lock = boost::lock_guard(entry->m_mutex);
    *snapshot = client->template SendRequest<
      LoadDirectoryEntryComplianceRuleEntryService>(directoryEntry);
    entry->m_queues.push_back(std::move(queue));
  }

  template<typename B>
  void ComplianceClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_complianceEntryQueues.With([&] (auto& entries) {
      for(auto& entry : entries | boost::adaptors::map_values) {
        for(auto& queue : entry->m_queues) {
          queue.Break();
        }
      }
      entries.clear();
    });
    m_openState.Close();
  }

  template<typename B>
  void ComplianceClient<B>::OnComplianceRuleEntry(ServiceProtocolClient& client,
      const ComplianceRuleEntry& entry) {
    auto queues = m_complianceEntryQueues.FindValue(entry.GetDirectoryEntry());
    if(!queues) {
      return;
    }
    auto lock = boost::lock_guard((*queues)->m_mutex);
    for(auto& queue : (*queues)->m_queues) {
      queue.Push(entry);
    }
  }
}

#endif
