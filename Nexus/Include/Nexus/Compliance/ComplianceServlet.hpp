#ifndef NEXUS_COMPLIANCE_SERVLET_HPP
#define NEXUS_COMPLIANCE_SERVLET_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Services/ServiceRequestException.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceServices.hpp"
#include "Nexus/Compliance/ComplianceSession.hpp"

namespace Nexus::Compliance {

  /**
   * Updates compliance rules and monitors violations.
   * @param C The container instantiating this servlet.
   * @param S The type of ServiceLocatorClient used to verify permissions.
   * @param A The type of AdministrationClient to use.
   * @param D The type of ComplianceRuleDataStore to use.
   * @param T The type of TimeClient used for timestamps.
   */
  template<typename C, typename S, typename A, typename D, typename T>
  class ComplianceServlet : private boost::noncopyable {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of ServiceLocatorClient used to verify permissions. */
      using ServiceLocatorClient = Beam::GetTryDereferenceType<S>;

      /** The type of AdministrationClient used to access account info. */
      using AdministrationClient = Beam::GetTryDereferenceType<A>;

      /** The type of ComplianceRuleDataStore to use. */
      using ComplianceRuleDataStore = Beam::GetTryDereferenceType<D>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::GetTryDereferenceType<T>;

      /**
       * Constructs a ComplianceServlet.
       * @param serviceLocatorClient Initializes the ServiceLocatorClient.
       * @param administrationClient Initializes the AdministrationClient.
       * @param dataStore Initializes the ComplianceRuleDataStore.
       * @param timeClient Initializes the TimeClient.
       */
      template<typename SF, typename AF, typename DF, typename TF>
      ComplianceServlet(SF&& serviceLocatorClient, AF&& administrationClient,
        DF&& dataStore, TF&& timeClient);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<S> m_serviceLocatorClient;
      Beam::GetOptionalLocalPtr<A> m_administrationClient;
      Beam::GetOptionalLocalPtr<D> m_dataStore;
      Beam::GetOptionalLocalPtr<T> m_timeClient;
      boost::atomic<ComplianceRuleId> m_nextEntryId;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        Beam::SynchronizedVector<ServiceProtocolClient*>>
        m_complianceEntrySubscriptions;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      std::vector<ComplianceRuleEntry> OnLoadDirectoryEntryComplianceRuleEntry(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);
      void OnMonitorComplianceRuleEntry(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);
      ComplianceRuleId OnAddComplianceRuleEntry(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);
      void OnUpdateComplianceRuleEntry(ServiceProtocolClient& client,
        const ComplianceRuleEntry& entry);
      void OnDeleteComplianceRuleEntry(ServiceProtocolClient& client,
        ComplianceRuleId id);
      void OnReportComplianceRuleViolation(ServiceProtocolClient& client,
        ComplianceRuleViolationRecord violationRecord);
  };

  template<typename S, typename A, typename D, typename T>
  struct MetaComplianceServlet {
    using Session = ComplianceSession;
    template<typename C>
    struct apply {
      using type = ComplianceServlet<C, S, A, D, T>;
    };
  };

  template<typename C, typename S, typename A, typename D, typename T>
  template<typename SF, typename AF, typename DF, typename TF>
  ComplianceServlet<C, S, A, D, T>::ComplianceServlet(SF&& serviceLocatorClient,
    AF&& administrationClient, DF&& dataStore, TF&& timeClient)
    : m_serviceLocatorClient(std::forward<SF>(serviceLocatorClient)),
      m_administrationClient(std::forward<AF>(administrationClient)),
      m_dataStore(std::forward<DF>(dataStore)),
      m_timeClient(std::forward<TF>(timeClient)) {}

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterComplianceServices(Store(slots));
    RegisterComplianceMessages(Store(slots));
    LoadDirectoryEntryComplianceRuleEntryService::AddSlot(Store(slots),
      std::bind(&ComplianceServlet::OnLoadDirectoryEntryComplianceRuleEntry,
      this, std::placeholders::_1, std::placeholders::_2));
    MonitorComplianceRuleEntryService::AddSlot(Store(slots),
      std::bind(&ComplianceServlet::OnMonitorComplianceRuleEntry, this,
      std::placeholders::_1, std::placeholders::_2));
    AddComplianceRuleEntryService::AddSlot(Store(slots), std::bind(
      &ComplianceServlet::OnAddComplianceRuleEntry, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    UpdateComplianceRuleEntryService::AddSlot(Store(slots), std::bind(
      &ComplianceServlet::OnUpdateComplianceRuleEntry, this,
      std::placeholders::_1, std::placeholders::_2));
    DeleteComplianceRuleEntryService::AddSlot(Store(slots), std::bind(
      &ComplianceServlet::OnDeleteComplianceRuleEntry, this,
      std::placeholders::_1, std::placeholders::_2));
    Beam::Services::AddMessageSlot<ReportComplianceRuleViolationMessage>(
      Beam::Store(slots), std::bind(
      &ComplianceServlet::OnReportComplianceRuleViolation, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::
      HandleClientClosed(ServiceProtocolClient& client) {
    m_complianceEntrySubscriptions.With(
      [&] (auto& subscriptions) {
        for(auto& subscription : subscriptions | boost::adaptors::map_values) {
          subscription.Remove(&client);
        }
      });
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorClient->Open();
      m_administrationClient->Open();
      m_dataStore->Open();
      m_nextEntryId = m_dataStore->LoadNextComplianceRuleEntryId();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::Shutdown() {
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename C, typename S, typename A, typename D, typename T>
  std::vector<ComplianceRuleEntry> ComplianceServlet<C, S, A, D, T>::
      OnLoadDirectoryEntryComplianceRuleEntry(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto& session = client.GetSession();
    auto hasPermission = m_serviceLocatorClient->HasPermissions(
      session.GetAccount(), directoryEntry,
      Beam::ServiceLocator::Permission::READ);
    if(!hasPermission) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return m_dataStore->LoadComplianceRuleEntries(directoryEntry);
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::OnMonitorComplianceRuleEntry(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto& session = client.GetSession();
    auto hasPermission = m_serviceLocatorClient->HasPermissions(
      session.GetAccount(), directoryEntry,
      Beam::ServiceLocator::Permission::READ);
    if(!hasPermission) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    m_complianceEntrySubscriptions.Get(directoryEntry).PushBack(&client);
  }

  template<typename C, typename S, typename A, typename D, typename T>
  ComplianceRuleId ComplianceServlet<C, S, A, D, T>::OnAddComplianceRuleEntry(
      ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto id = ++m_nextEntryId;
    auto entry = ComplianceRuleEntry(id, directoryEntry, state, schema);
    m_dataStore->Store(entry);
    auto& subscribers = m_complianceEntrySubscriptions.Get(directoryEntry);
    subscribers.ForEach(
      [&] (auto client) {
        Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(*client,
          entry);
      });
    return entry.GetId();
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::OnUpdateComplianceRuleEntry(
      ServiceProtocolClient& client, const ComplianceRuleEntry& entry) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    m_dataStore->Store(entry);
    auto& subscribers = m_complianceEntrySubscriptions.Get(
      entry.GetDirectoryEntry());
    subscribers.ForEach(
      [&] (auto client) {
        Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(*client,
          entry);
      });
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::OnDeleteComplianceRuleEntry(
      ServiceProtocolClient& client, ComplianceRuleId id) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto entry = m_dataStore->LoadComplianceRuleEntry(id);
    m_dataStore->Delete(id);
    if(!entry.is_initialized()) {
      return;
    }
    entry->SetState(ComplianceRuleEntry::State::DELETED);
    auto& subscribers = m_complianceEntrySubscriptions.Get(
      entry->GetDirectoryEntry());
    subscribers.ForEach(
      [&] (auto client) {
        Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(*client,
          *entry);
      });
  }

  template<typename C, typename S, typename A, typename D, typename T>
  void ComplianceServlet<C, S, A, D, T>::OnReportComplianceRuleViolation(
      ServiceProtocolClient& client,
      ComplianceRuleViolationRecord violationRecord) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    violationRecord.m_timestamp = m_timeClient->GetTime();
    m_dataStore->Store(violationRecord);
  }
}

#endif
