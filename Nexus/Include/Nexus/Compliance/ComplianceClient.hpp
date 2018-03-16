#ifndef NEXUS_COMPLIANCECLIENT_HPP
#define NEXUS_COMPLIANCECLIENT_HPP
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Services/ServiceProtocolClient.hpp>
#include <Beam/Threading/CallOnce.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/functional/factory.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceServices.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class ComplianceClient
      \brief Client used to access the Compliance service.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class ComplianceClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder =
        Beam::GetTryDereferenceType<ServiceProtocolClientBuilderType>;

      //! Constructs a ComplianceClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      ComplianceClient(ClientBuilderForward&& clientBuilder);

      ~ComplianceClient();

      //! Loads all ComplianceRuleEntries for a specified DirectoryEntry.
      /*!
        \param directoryEntry The DirectoryEntry to query.
        \return The list of all ComplianceRuleEntries assigned to the
                <i>directoryEntry</i>.
      */
      std::vector<ComplianceRuleEntry> Load(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

      //! Assigns a new compliance rule to a DirectoryEntry.
      /*!
        \param directoryEntry The DirectoryEntry to assign the rule to.
        \param state The rule's initial State.
        \param schema The ComplianceRuleSchema specifying the rule to add.
        \return The id of the new entry.
      */
      ComplianceRuleId Add(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);

      //! Updates an existing compliance rule.
      /*!
        \param entry The ComplianceRuleEntry to update.
      */
      void Update(const ComplianceRuleEntry& entry);

      //! Deletes a ComplianceRuleEntry.
      /*!
        \param id The ComplianceRuleId to delete.
      */
      void Delete(ComplianceRuleId id);

      //! Reports a compliance violation.
      /*!
        \param violationRecord The violation to report.
      */
      void Report(const ComplianceRuleViolationRecord& violationRecord);

      //! Monitors updates to a DirectoryEntry's ComplianceRuleEntries.
      /*!
        \param directoryEntry The DirectoryEntry to monitor.
        \param queue Stores the changes to the <i>directoryEntry</i>'s
               ComplianceRuleEntries.
        \param snapshot Stores the snapshot of the current
               ComplianceRuleEntries.
      */
      void MonitorComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);

      void Open();

      void Close();

    private:
      struct ComplianceQueueEntry {
        Beam::Threading::Mutex m_mutex;
        std::vector<std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>>
          m_queues;
        Beam::Threading::CallOnce<Beam::Threading::Mutex> m_initializer;
      };
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<ComplianceQueueEntry>> m_complianceEntryQueues;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnComplianceRuleEntry(ServiceProtocolClient& client,
        const ComplianceRuleEntry& entry);
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  ComplianceClient<ServiceProtocolClientBuilderType>::ComplianceClient(
      ClientBuilderForward&& clientBuilder)
      : m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)) {
    RegisterComplianceServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterComplianceMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<ComplianceRuleEntryMessage>(
      Store(m_clientHandler.GetSlots()),
      std::bind(&ComplianceClient::OnComplianceRuleEntry, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename ServiceProtocolClientBuilderType>
  ComplianceClient<ServiceProtocolClientBuilderType>::~ComplianceClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<ComplianceRuleEntry>
      ComplianceClient<ServiceProtocolClientBuilderType>::Load(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadDirectoryEntryComplianceRuleEntryService>(directoryEntry);
  }

  template<typename ServiceProtocolClientBuilderType>
  ComplianceRuleId ComplianceClient<ServiceProtocolClientBuilderType>::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<AddComplianceRuleEntryService>(
      directoryEntry, state, schema);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::Update(
      const ComplianceRuleEntry& entry) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<UpdateComplianceRuleEntryService>(entry);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::Delete(
      ComplianceRuleId id) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<DeleteComplianceRuleEntryService>(id);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    auto client = m_clientHandler.GetClient();
    Beam::Services::SendRecordMessage<ReportComplianceRuleViolationMessage>(
      *client, violationRecord);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::
      MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    auto entry = m_complianceEntryQueues.GetOrInsert(directoryEntry,
      boost::factory<std::shared_ptr<ComplianceQueueEntry>>());
    auto client = m_clientHandler.GetClient();
    entry->m_initializer.Call(
      [&] {
        client->template SendRequest<MonitorComplianceRuleEntryService>(
          directoryEntry);
      });
    boost::lock_guard<Beam::Threading::Mutex> lock{entry->m_mutex};
    *snapshot = client->template SendRequest<
      LoadDirectoryEntryComplianceRuleEntryService>(directoryEntry);
    entry->m_queues.push_back(queue);
  }

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::Open() {
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

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_tasks.Break();
    m_complianceEntryQueues.With(
      [&] (std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
          std::shared_ptr<ComplianceQueueEntry>>& entries) {
        for(auto& entry : entries | boost::adaptors::map_values) {
          for(auto& queue : entry->m_queues) {
            queue->Break();
          }
        }
        entries.clear();
      });
    m_openState.SetClosed();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ComplianceClient<ServiceProtocolClientBuilderType>::
      OnComplianceRuleEntry(ServiceProtocolClient& client,
      const ComplianceRuleEntry& entry) {
    auto queues = m_complianceEntryQueues.FindValue(entry.GetDirectoryEntry());
    if(!queues.is_initialized()) {
      return;
    }
    boost::lock_guard<Beam::Threading::Mutex> lock{(*queues)->m_mutex};
    for(auto& queue : (*queues)->m_queues) {
      queue->Push(entry);
    }
  }
}
}

#endif
