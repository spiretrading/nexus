#ifndef NEXUS_COMPLIANCE_CLIENT_HPP
#define NEXUS_COMPLIANCE_CLIENT_HPP
#include <memory>
#include <utility>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus {

  /** Provides a generic interface over an arbitrary ComplianceClient. */
  class ComplianceClient {
    public:

      /**
       * Constructs a ComplianceClient of a specified type using emplacement.
       * @param <T> The type of compliance client to emplace.
       * @param args The arguments to pass to the emplaced compliance client.
       */
      template<typename T, typename... Args>
      explicit ComplianceClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ComplianceClient by copying an existing compliance client.
       * @param client The client to copy.
       */
      template<typename C>
      explicit ComplianceClient(C client);

      explicit ComplianceClient(ComplianceClient* client);

      explicit ComplianceClient(
        const std::shared_ptr<ComplianceClient>& client);

      explicit ComplianceClient(
        const std::unique_ptr<ComplianceClient>& client);

      /**
       * Loads all ComplianceRuleEntries for a specified DirectoryEntry.
       * @param directory_entry The DirectoryEntry to query.
       * @return The list of all ComplianceRuleEntries assigned to the
       *         <i>entry</i>.
       */
      std::vector<ComplianceRuleEntry> load(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry);

      /**
       * Assigns a new compliance rule to a DirectoryEntry.
       * @param directory_entry The DirectoryEntry to assign the rule to.
       * @param state The rule's initial State.
       * @param schema The ComplianceRuleSchema specifying the rule to add.
       * @return The id of the new entry.
       */
      ComplianceRuleEntry::Id add(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);

      /**
       * Updates an existing compliance rule.
       * @param entry The ComplianceRuleEntry to update.
       */
      void update(const ComplianceRuleEntry& entry);

      /**
       * Deletes a ComplianceRuleEntry.
       * @param id The ComplianceRuleId to delete.
       */
      void remove(ComplianceRuleEntry::Id id);

      /**
       * Reports a compliance violation.
       * @param record The violation to report.
       */
      void report(const ComplianceRuleViolationRecord& record);

      /**
       * Monitors updates to a DirectoryEntry's ComplianceRuleEntries.
       * @param directory_entry The DirectoryEntry to monitor.
       * @param queue Stores the changes to the <i>directoryEntry</i>'s
       *        ComplianceRuleEntries.
       * @param snapshot Stores the snapshot of the current
       *        ComplianceRuleEntries.
       */
      void monitor_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);

      void close();

    private:
      struct VirtualComplianceClient {
        virtual ~VirtualComplianceClient() = default;
        virtual std::vector<ComplianceRuleEntry> load(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry) = 0;
        virtual ComplianceRuleEntry::Id add(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry,
          ComplianceRuleEntry::State state,
          const ComplianceRuleSchema& schema) = 0;
        virtual void update(const ComplianceRuleEntry& entry) = 0;
        virtual void remove(ComplianceRuleEntry::Id id) = 0;
        virtual void report(const ComplianceRuleViolationRecord& record) = 0;
        virtual void monitor_compliance_rule_entries(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry,
          Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
          Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedComplianceClient final : VirtualComplianceClient {
        using ComplianceClient = C;
        Beam::GetOptionalLocalPtr<ComplianceClient> m_client;

        template<typename... Args>
        explicit WrappedComplianceClient(Args&&... args);
        std::vector<ComplianceRuleEntry> load(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry) override;
        ComplianceRuleEntry::Id add(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry,
          ComplianceRuleEntry::State state,
          const ComplianceRuleSchema& schema) override;
        void update(const ComplianceRuleEntry& entry) override;
        void remove(ComplianceRuleEntry::Id id) override;
        void report(const ComplianceRuleViolationRecord& record) override;
        void monitor_compliance_rule_entries(
          const Beam::ServiceLocator::DirectoryEntry& directory_entry,
          Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
          Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) override;
        void close() override;
      };
      std::shared_ptr<VirtualComplianceClient> m_client;
  };

  /** Checks if a type implements a ComplianceClient. */
  template<typename T>
  concept IsComplianceClient = std::constructible_from<
    ComplianceClient, std::remove_pointer_t<std::remove_cvref_t<T>>*>;

  template<typename T, typename... Args>
  ComplianceClient::ComplianceClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedComplianceClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename C>
  ComplianceClient::ComplianceClient(C client)
    : ComplianceClient(std::in_place_type<C>, std::move(client)) {}

  inline ComplianceClient::ComplianceClient(ComplianceClient* client)
    : ComplianceClient(*client) {}

  inline ComplianceClient::ComplianceClient(
    const std::shared_ptr<ComplianceClient>& client)
    : ComplianceClient(*client) {}

  inline ComplianceClient::ComplianceClient(
    const std::unique_ptr<ComplianceClient>& client)
    : ComplianceClient(*client) {}

  inline std::vector<ComplianceRuleEntry> ComplianceClient::load(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    return m_client->load(directory_entry);
  }

  inline ComplianceRuleEntry::Id ComplianceClient::add(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return m_client->add(directory_entry, state, schema);
  }

  inline void ComplianceClient::update(const ComplianceRuleEntry& entry) {
    return m_client->update(entry);
  }

  inline void ComplianceClient::remove(ComplianceRuleEntry::Id id) {
    return m_client->remove(id);
  }

  inline void ComplianceClient::report(
      const ComplianceRuleViolationRecord& record) {
    return m_client->report(record);
  }

  inline void ComplianceClient::monitor_compliance_rule_entries(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return m_client->monitor_compliance_rule_entries(
      directory_entry, std::move(queue), Beam::Store(snapshot));
  }

  inline void ComplianceClient::close() {
    return m_client->close();
  }

  template<typename C>
  template<typename... Args>
  ComplianceClient::WrappedComplianceClient<C>::WrappedComplianceClient(
    Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::vector<ComplianceRuleEntry>
  ComplianceClient::WrappedComplianceClient<C>::load(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    return m_client->load(directory_entry);
  }

  template<typename C>
  ComplianceRuleEntry::Id ComplianceClient::WrappedComplianceClient<C>::add(
      const Beam::ServiceLocator::DirectoryEntry& directory_entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return m_client->add(directory_entry, state, schema);
  }

  template<typename C>
  void ComplianceClient::WrappedComplianceClient<C>::update(
      const ComplianceRuleEntry& entry) {
    return m_client->update(entry);
  }

  template<typename C>
  void ComplianceClient::WrappedComplianceClient<C>::remove(
      ComplianceRuleEntry::Id id) {
    return m_client->remove(id);
  }

  template<typename C>
  void ComplianceClient::WrappedComplianceClient<C>::report(
      const ComplianceRuleViolationRecord& record) {
    return m_client->report(record);
  }

  template<typename C>
  void ComplianceClient::WrappedComplianceClient<C>::
      monitor_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return m_client->monitor_compliance_rule_entries(
      directory_entry, std::move(queue), Beam::Store(snapshot));
  }

  template<typename C>
  void ComplianceClient::WrappedComplianceClient<C>::close() {
    return m_client->close();
  }
}

#endif
