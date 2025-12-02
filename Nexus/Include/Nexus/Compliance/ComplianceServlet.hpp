#ifndef NEXUS_COMPLIANCE_SERVLET_HPP
#define NEXUS_COMPLIANCE_SERVLET_HPP
#include <atomic>
#include <ranges>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/Services/ServiceRequestException.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceServices.hpp"
#include "Nexus/Compliance/ComplianceSession.hpp"

namespace Nexus {

  /**
   * Updates compliance rules and monitors violations.
   * @param C The container instantiating this servlet.
   * @param S The type of ServiceLocatorClient used to verify permissions.
   * @param A The type of AdministrationClient to use.
   * @param D The type of ComplianceRuleDataStore to use.
   * @param T The type of TimeClient used for timestamps.
   */
  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  class ComplianceServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of ServiceLocatorClient used to verify permissions. */
      using ServiceLocatorClient = Beam::dereference_t<S>;

      /** The type of AdministrationClient used to access account info. */
      using AdministrationClient = Beam::dereference_t<A>;

      /** The type of ComplianceRuleDataStore to use. */
      using ComplianceRuleDataStore = Beam::dereference_t<D>;

      /** The type of TimeClient to use. */
      using TimeClient = Beam::dereference_t<T>;

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

      void register_services(
        Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      Beam::local_ptr_t<S> m_service_locator_client;
      Beam::local_ptr_t<A> m_administration_client;
      Beam::local_ptr_t<D> m_data_store;
      Beam::local_ptr_t<T> m_time_client;
      std::atomic<ComplianceRuleEntry::Id> m_next_entry_id;
      Beam::SynchronizedUnorderedMap<Beam::DirectoryEntry,
        Beam::SynchronizedVector<ServiceProtocolClient*>> m_subscriptions;
      Beam::OpenState m_open_state;

      ComplianceServlet(const ComplianceServlet&) = delete;
      ComplianceServlet& operator =(const ComplianceServlet&) = delete;
      std::vector<ComplianceRuleEntry>
        on_load_directory_entry_compliance_rule_entry(
          ServiceProtocolClient& client,
          const Beam::DirectoryEntry& directory_entry);
      std::vector<ComplianceRuleEntry> on_monitor_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::DirectoryEntry& directory_entry);
      ComplianceRuleEntry::Id on_add_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);
      void on_update_compliance_rule_entry(
        ServiceProtocolClient& client, const ComplianceRuleEntry& entry);
      void on_delete_compliance_rule_entry(
        ServiceProtocolClient& client, ComplianceRuleEntry::Id id);
      void on_report_compliance_rule_violation(
        ServiceProtocolClient& client, ComplianceRuleViolationRecord record);
  };

  template<typename S, typename A, typename D, typename T>
  struct MetaComplianceServlet {
    using Session = ComplianceSession;

    template<typename C>
    struct apply {
      using type = ComplianceServlet<C, S, A, D, T>;
    };
  };

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
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
      close();
      throw;
    }
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  void ComplianceServlet<C, S, A, D, T>::register_services(
      Beam::Out<Beam::ServiceSlots<ServiceProtocolClient>> slots) {
    register_compliance_services(out(slots));
    register_compliance_messages(out(slots));
    LoadDirectoryEntryComplianceRuleEntryService::add_slot(out(slots),
      std::bind_front(
        &ComplianceServlet::on_load_directory_entry_compliance_rule_entry,
          this));
    MonitorComplianceRuleEntryService::add_slot(out(slots),
      std::bind_front(&ComplianceServlet::on_monitor_compliance_rule_entry,
        this));
    AddComplianceRuleEntryService::add_slot(out(slots),
      std::bind_front(&ComplianceServlet::on_add_compliance_rule_entry, this));
    UpdateComplianceRuleEntryService::add_slot(out(slots),
      std::bind_front(&ComplianceServlet::on_update_compliance_rule_entry,
        this));
    DeleteComplianceRuleEntryService::add_slot(out(slots),
      std::bind_front(&ComplianceServlet::on_delete_compliance_rule_entry,
        this));
    Beam::add_message_slot<ReportComplianceRuleViolationMessage>(out(slots),
      std::bind_front(
        &ComplianceServlet::on_report_compliance_rule_violation, this));
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  void ComplianceServlet<C, S, A, D, T>::handle_close(
      ServiceProtocolClient& client) {
    m_subscriptions.for_each_value([&] (auto& subscription) {
      subscription.erase(&client);
    });
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  void ComplianceServlet<C, S, A, D, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_open_state.close();
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  std::vector<ComplianceRuleEntry> ComplianceServlet<C, S, A, D, T>::
      on_load_directory_entry_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::DirectoryEntry& directory_entry) {
    auto& session = client.get_session();
    auto has_permission = m_service_locator_client->has_permissions(
      session.get_account(), directory_entry, Beam::Permission::READ);
    if(!has_permission) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  std::vector<ComplianceRuleEntry>
      ComplianceServlet<C, S, A, D, T>::on_monitor_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::DirectoryEntry& directory_entry) {
    auto& session = client.get_session();
    auto has_permission = m_service_locator_client->has_permissions(
      session.get_account(), directory_entry, Beam::Permission::READ);
    if(!has_permission) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    m_subscriptions.get(directory_entry).push_back(&client);
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  ComplianceRuleEntry::Id
      ComplianceServlet<C, S, A, D, T>::on_add_compliance_rule_entry(
        ServiceProtocolClient& client,
        const Beam::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto& session = client.get_session();
    if(!m_administration_client->check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto id = ++m_next_entry_id;
    auto entry = ComplianceRuleEntry(id, directory_entry, state, schema);
    m_data_store->store(entry);
    auto& subscribers = m_subscriptions.get(directory_entry);
    subscribers.for_each([&] (auto client) {
      Beam::send_record_message<ComplianceRuleEntryMessage>(*client, entry);
    });
    return entry.get_id();
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  void ComplianceServlet<C, S, A, D, T>::on_update_compliance_rule_entry(
      ServiceProtocolClient& client, const ComplianceRuleEntry& entry) {
    auto& session = client.get_session();
    if(!m_administration_client->check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    m_data_store->store(entry);
    auto& subscribers = m_subscriptions.get(entry.get_directory_entry());
    subscribers.for_each([&] (auto client) {
      Beam::send_record_message<ComplianceRuleEntryMessage>(*client, entry);
    });
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  void ComplianceServlet<C, S, A, D, T>::on_delete_compliance_rule_entry(
      ServiceProtocolClient& client, ComplianceRuleEntry::Id id) {
    auto& session = client.get_session();
    if(!m_administration_client->check_administrator(session.get_account())) {
      boost::throw_with_location(
        Beam::ServiceRequestException("Insufficient permissions."));
    }
    auto entry = m_data_store->load_compliance_rule_entry(id);
    m_data_store->remove(id);
    if(!entry) {
      return;
    }
    entry->set_state(ComplianceRuleEntry::State::DELETED);
    auto& subscribers = m_subscriptions.get(entry->get_directory_entry());
    subscribers.for_each([&] (auto client) {
      Beam::send_record_message<ComplianceRuleEntryMessage>(*client, *entry);
    });
  }

  template<typename C, typename S, typename A, typename D, typename T> requires
    Beam::IsServiceLocatorClient<Beam::dereference_t<S>> &&
      IsAdministrationClient<Beam::dereference_t<A>> &&
        IsComplianceRuleDataStore<Beam::dereference_t<D>> &&
          Beam::IsTimeClient<Beam::dereference_t<T>>
  void ComplianceServlet<C, S, A, D, T>::on_report_compliance_rule_violation(
      ServiceProtocolClient& client, ComplianceRuleViolationRecord record) {
    auto& session = client.get_session();
    if(!m_administration_client->check_administrator(session.get_account())) {
      return;
    }
    record.m_timestamp = m_time_client->get_time();
    m_data_store->store(record);
  }
}

#endif
