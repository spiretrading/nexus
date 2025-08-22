#ifndef NEXUS_COMPLIANCE_SERVLET_HPP
#define NEXUS_COMPLIANCE_SERVLET_HPP
#include <atomic>
#include <ranges>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Services/ServiceRequestException.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
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
  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  class ComplianceServlet {
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
       * @param service_locator_client Initializes the ServiceLocatorClient.
       * @param administration_client Initializes the AdministrationClient.
       * @param data_store Initializes the ComplianceRuleDataStore.
       * @param time_client Initializes the TimeClient.
       */
      template<Beam::Initializes<S> SF, Beam::Initializes<A> AF,
        Beam::Initializes<D> DF, Beam::Initializes<T> TF>
      ComplianceServlet(SF&& service_locator_client, AF&& administration_client,
        DF&& data_store, TF&& time_client);

      void RegisterServices(
        Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<S> m_service_locator_client;
      Beam::GetOptionalLocalPtr<A> m_administration_client;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      Beam::GetOptionalLocalPtr<T> m_time_client;
      std::atomic<ComplianceRuleEntry::Id> m_next_entry_id;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        Beam::SynchronizedVector<ServiceProtocolClient*>> m_subscriptions;
      Beam::IO::OpenState m_open_state;

      ComplianceServlet(const ComplianceServlet&) = delete;
      ComplianceServlet& operator =(const ComplianceServlet&) = delete;
      std::vector<ComplianceRuleEntry>
        on_load_directory_entry_compliance_rule_entry(
          ServiceProtocolClient& client,
          const Beam::ServiceLocator::DirectoryEntry& directory_entry);
      std::vector<ComplianceRuleEntry> on_monitor_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directory_entry);
      ComplianceRuleEntry::Id on_add_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);
      void on_update_compliance_rule_entry(
        ServiceProtocolClient& client, const ComplianceRuleEntry& entry);
      void on_delete_compliance_rule_entry(
        ServiceProtocolClient& client, ComplianceRuleEntry::Id id);
      void on_report_compliance_rule_violation(
        ServiceProtocolClient& client, ComplianceRuleViolationRecord record);
  };

  template<typename S, IsAdministrationClient A, IsComplianceRuleDataStore D,
    typename T>
  struct MetaComplianceServlet {
    using Session = ComplianceSession;
    template<typename C>
    struct apply {
      using type = ComplianceServlet<C, S, A, D, T>;
    };
  };

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  template<Beam::Initializes<S> SF, Beam::Initializes<A> AF,
    Beam::Initializes<D> DF, Beam::Initializes<T> TF>
  ComplianceServlet<C, S, A, D, T>::ComplianceServlet(
      SF&& service_locator_client, AF&& administration_client, DF&& data_store,
      TF&& time_client)
      : m_service_locator_client(std::forward<SF>(service_locator_client)),
        m_administration_client(std::forward<AF>(administration_client)),
        m_data_store(std::forward<DF>(data_store)),
        m_time_client(std::forward<TF>(time_client)) {
    try {
      m_next_entry_id = m_data_store->load_next_compliance_rule_entry_id();
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  void ComplianceServlet<C, S, A, D, T>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterComplianceServices(Store(slots));
    RegisterComplianceMessages(Store(slots));
    LoadDirectoryEntryComplianceRuleEntryService::AddSlot(
      Store(slots), std::bind_front(
        &ComplianceServlet::on_load_directory_entry_compliance_rule_entry,
        this));
    MonitorComplianceRuleEntryService::AddSlot(Store(slots), std::bind_front(
      &ComplianceServlet::on_monitor_compliance_rule_entry, this));
    AddComplianceRuleEntryService::AddSlot(Store(slots),
      std::bind_front(&ComplianceServlet::on_add_compliance_rule_entry, this));
    UpdateComplianceRuleEntryService::AddSlot(Store(slots), std::bind_front(
      &ComplianceServlet::on_update_compliance_rule_entry, this));
    DeleteComplianceRuleEntryService::AddSlot(Store(slots), std::bind_front(
      &ComplianceServlet::on_delete_compliance_rule_entry, this));
    Beam::Services::AddMessageSlot<ReportComplianceRuleViolationMessage>(
      Store(slots), std::bind_front(
        &ComplianceServlet::on_report_compliance_rule_violation, this));
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  void ComplianceServlet<C, S, A, D, T>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_subscriptions.With([&] (auto& subscriptions) {
      for(auto& subscription : subscriptions | std::views::values) {
        subscription.Remove(&client);
      }
    });
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  void ComplianceServlet<C, S, A, D, T>::Close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_open_state.Close();
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  std::vector<ComplianceRuleEntry> ComplianceServlet<C, S, A, D, T>::
      on_load_directory_entry_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    auto& session = client.GetSession();
    auto has_permission = m_service_locator_client->HasPermissions(
      session.GetAccount(), directory_entry,
      Beam::ServiceLocator::Permission::READ);
    if(!has_permission) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  std::vector<ComplianceRuleEntry>
      ComplianceServlet<C, S, A, D, T>::on_monitor_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    auto& session = client.GetSession();
    auto has_permission = m_service_locator_client->HasPermissions(
      session.GetAccount(), directory_entry,
      Beam::ServiceLocator::Permission::READ);
    if(!has_permission) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    m_subscriptions.Get(directory_entry).PushBack(&client);
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  ComplianceRuleEntry::Id
      ComplianceServlet<C, S, A, D, T>::on_add_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto& session = client.GetSession();
    auto is_administrator = m_administration_client->check_administrator(
      session.GetAccount());
    if(!is_administrator) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto id = ++m_next_entry_id;
    auto entry = ComplianceRuleEntry(id, directory_entry, state, schema);
    m_data_store->store(entry);
    auto& subscribers = m_subscriptions.Get(directory_entry);
    subscribers.ForEach([&] (auto client) {
      Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(
        *client, entry);
    });
    return entry.get_id();
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  void ComplianceServlet<C, S, A, D, T>::on_update_compliance_rule_entry(
      ServiceProtocolClient& client, const ComplianceRuleEntry& entry) {
    auto& session = client.GetSession();
    auto is_administrator = m_administration_client->check_administrator(
      session.GetAccount());
    if(!is_administrator) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    m_data_store->store(entry);
    auto& subscribers = m_subscriptions.Get(entry.get_directory_entry());
    subscribers.ForEach([&] (auto client) {
      Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(
        *client, entry);
    });
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  void ComplianceServlet<C, S, A, D, T>::on_delete_compliance_rule_entry(
      ServiceProtocolClient& client, ComplianceRuleEntry::Id id) {
    auto& session = client.GetSession();
    auto is_administrator = m_administration_client->check_administrator(
      session.GetAccount());
    if(!is_administrator) {
      throw Beam::Services::ServiceRequestException(
        "Insufficient permissions.");
    }
    auto entry = m_data_store->load_compliance_rule_entry(id);
    m_data_store->remove(id);
    if(!entry) {
      return;
    }
    entry->set_state(ComplianceRuleEntry::State::DELETED);
    auto& subscribers = m_subscriptions.Get(entry->get_directory_entry());
    subscribers.ForEach([&] (auto client) {
      Beam::Services::SendRecordMessage<ComplianceRuleEntryMessage>(
        *client, *entry);
    });
  }

  template<typename C, typename S, IsAdministrationClient A,
    IsComplianceRuleDataStore D, typename T>
  void ComplianceServlet<C, S, A, D, T>::on_report_compliance_rule_violation(
      ServiceProtocolClient& client, ComplianceRuleViolationRecord record) {
    auto& session = client.GetSession();
    auto is_administrator = m_administration_client->check_administrator(
      session.GetAccount());
    if(!is_administrator) {
      return;
    }
    record.m_timestamp = m_time_client->GetTime();
    m_data_store->store(record);
  }
}

#endif
