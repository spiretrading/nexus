#ifndef NEXUS_COMPLIANCESERVLET_HPP
#define NEXUS_COMPLIANCESERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Services/ServiceRequestException.hpp>
#include <Beam/Utilities/SynchronizedList.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceServices.hpp"
#include "Nexus/Compliance/ComplianceSession.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class ComplianceServlet
      \brief Updates compliance rules and monitors violations.
      \tparam ContainerType The container instantiating this servlet.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient used to
              verify permissions.
      \tparam AdministrationClientType The type of AdministrationClient to use.
      \tparam ComplianceRuleDataStoreType The type of ComplianceRuleDataStore to
              use.
      \tparam TimeClientType The type of TimeClient used for timestamps.
   */
  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  class ComplianceServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of ServiceLocatorClient used to verify permissions.
      using ServiceLocatorClient =
        Beam::GetTryDereferenceType<ServiceLocatorClientType>;

      //! The type of AdministrationClient used to access account info.
      using AdministrationClient =
        Beam::GetTryDereferenceType<AdministrationClientType>;

      //! The type of ComplianceRuleDataStore to use.
      using ComplianceRuleDataStore = Beam::GetTryDereferenceType<
        ComplianceRuleDataStoreType>;

      //! The type of TimeClient to use.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs a ComplianceServlet.
      /*!
        \param serviceLocatorClient Initializes the ServiceLocatorClient.
        \param administrationClient Initializes the AdministrationClient.
        \param dataStore Initializes the ComplianceRuleDataStore.
        \param timeClient Initializes the TimeClient.
      */
      template<typename ServiceLocatorClientForward,
        typename AdministrationClientForward,
        typename ComplianceRuleDataStoreForward, typename TimeClientForward>
      ComplianceServlet(ServiceLocatorClientForward&& serviceLocatorClient,
        AdministrationClientForward&& administrationClient,
        ComplianceRuleDataStoreForward&& dataStore,
        TimeClientForward&& timeClient);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<ServiceLocatorClientType>
        m_serviceLocatorClient;
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::GetOptionalLocalPtr<ComplianceRuleDataStoreType> m_dataStore;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
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

  template<typename ServiceLocatorClientType, typename AdministrationClientType,
    typename ComplianceRuleDataStoreType, typename TimeClientType>
  struct MetaComplianceServlet {
    using Session = ComplianceSession;
    template<typename ContainerType>
    struct apply {
      using type = ComplianceServlet<ContainerType, ServiceLocatorClientType,
        AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>;
    };
  };

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  template<typename ServiceLocatorClientForward,
    typename AdministrationClientForward,
    typename ComplianceRuleDataStoreForward, typename TimeClientForward>
  ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      ComplianceServlet(ServiceLocatorClientForward&& serviceLocatorClient,
      AdministrationClientForward&& administrationClient,
      ComplianceRuleDataStoreForward&& dataStore,
      TimeClientForward&& timeClient)
      : m_serviceLocatorClient{std::forward<ServiceLocatorClientForward>(
          serviceLocatorClient)},
        m_administrationClient{std::forward<AdministrationClientForward>(
          administrationClient)},
        m_dataStore{std::forward<ComplianceRuleDataStoreForward>(dataStore)},
        m_timeClient{std::forward<TimeClientForward>(timeClient)} {}

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      RegisterServices(
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

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      HandleClientClosed(ServiceProtocolClient& client) {
    m_complianceEntrySubscriptions.With(
      [&] (std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
          Beam::SynchronizedVector<ServiceProtocolClient*>>& subscriptions) {
        for(auto& subscription : subscriptions | boost::adaptors::map_values) {
          subscription.Remove(&client);
        }
      });
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      Open() {
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

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      Shutdown() {
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  std::vector<ComplianceRuleEntry> ComplianceServlet<ContainerType,
      ServiceLocatorClientType, AdministrationClientType,
      ComplianceRuleDataStoreType, TimeClientType>::
      OnLoadDirectoryEntryComplianceRuleEntry(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto& session = client.GetSession();
    auto hasPermission = m_serviceLocatorClient->HasPermissions(
      session.GetAccount(), directoryEntry,
      Beam::ServiceLocator::Permission::READ);
    if(!hasPermission) {
      throw Beam::Services::ServiceRequestException{
        "Insufficient permissions."};
    }
    auto entry = m_dataStore->LoadComplianceRuleEntries(directoryEntry);
    return entry;
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      OnMonitorComplianceRuleEntry(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto& session = client.GetSession();
    auto hasPermission = m_serviceLocatorClient->HasPermissions(
      session.GetAccount(), directoryEntry,
      Beam::ServiceLocator::Permission::READ);
    if(!hasPermission) {
      throw Beam::Services::ServiceRequestException{
        "Insufficient permissions."};
    }
    m_complianceEntrySubscriptions.Get(directoryEntry).PushBack(&client);
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  ComplianceRuleId ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      OnAddComplianceRuleEntry(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException{
        "Insufficient permissions."};
    }
    auto id = ++m_nextEntryId;
    ComplianceRuleEntry entry{id, directoryEntry, state, schema};
    m_dataStore->Store(entry);
    auto& subscribers = m_complianceEntrySubscriptions.Get(directoryEntry);
    subscribers.ForEach(
      [&] (ServiceProtocolClient* client) {
        Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(*client,
          entry);
      });
    return entry.GetId();
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      OnUpdateComplianceRuleEntry(ServiceProtocolClient& client,
      const ComplianceRuleEntry& entry) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException{
        "Insufficient permissions."};
    }
    m_dataStore->Store(entry);
    auto& subscribers = m_complianceEntrySubscriptions.Get(
      entry.GetDirectoryEntry());
    subscribers.ForEach(
      [&] (ServiceProtocolClient* client) {
        Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(*client,
          entry);
      });
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      OnDeleteComplianceRuleEntry(ServiceProtocolClient& client,
      ComplianceRuleId id) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException{
        "Insufficient permissions."};
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
      [&] (ServiceProtocolClient* client) {
        Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(*client,
          *entry);
      });
  }

  template<typename ContainerType, typename ServiceLocatorClientType,
    typename AdministrationClientType, typename ComplianceRuleDataStoreType,
    typename TimeClientType>
  void ComplianceServlet<ContainerType, ServiceLocatorClientType,
      AdministrationClientType, ComplianceRuleDataStoreType, TimeClientType>::
      OnReportComplianceRuleViolation(ServiceProtocolClient& client,
      ComplianceRuleViolationRecord violationRecord) {
    auto& session = client.GetSession();
    auto isAdministrator = m_administrationClient->CheckAdministrator(
      session.GetAccount());
    if(!isAdministrator) {
      throw Beam::Services::ServiceRequestException{
        "Insufficient permissions."};
    }
    violationRecord.m_timestamp = m_timeClient->GetTime();
    m_dataStore->Store(violationRecord);
  }
}
}

#endif
