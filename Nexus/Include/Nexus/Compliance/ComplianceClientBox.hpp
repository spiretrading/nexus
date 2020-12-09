#ifndef NEXUS_COMPLIANCE_CLIENT_BOX_HPP
#define NEXUS_COMPLIANCE_CLIENT_BOX_HPP
#include <memory>
#include <type_traits>
#include <vector>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus::Compliance {

  /** Provides a generic interface over an arbitrary ComplianceClient. */
  class ComplianceClientBox {
    public:

      /**
       * Constructs a ComplianceClientBox of a specified type using emplacement.
       * @param <T> The type of compliance client to emplace.
       * @param args The arguments to pass to the emplaced compliance client.
       */
      template<typename T, typename... Args>
      explicit ComplianceClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ComplianceClientBox by copying an existing compliance
       * client.
       * @param client The client to copy.
       */
      template<typename ComplianceClient>
      explicit ComplianceClientBox(ComplianceClient client);

      explicit ComplianceClientBox(ComplianceClientBox* client);

      explicit ComplianceClientBox(
        const std::shared_ptr<ComplianceClientBox>& client);

      explicit ComplianceClientBox(
        const std::unique_ptr<ComplianceClientBox>& client);

      std::vector<ComplianceRuleEntry> Load(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

      ComplianceRuleId Add(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
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
      struct VirtualComplianceClient {
        virtual ~VirtualComplianceClient() = default;
        virtual std::vector<ComplianceRuleEntry> Load(
          const Beam::ServiceLocator::DirectoryEntry& directoryEntry) = 0;
        virtual ComplianceRuleId Add(
          const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
          ComplianceRuleEntry::State state,
          const ComplianceRuleSchema& schema) = 0;
        virtual void Update(const ComplianceRuleEntry& entry) = 0;
        virtual void Delete(ComplianceRuleId id) = 0;
        virtual void Report(
          const ComplianceRuleViolationRecord& violationRecord) = 0;
        virtual void MonitorComplianceRuleEntries(
          const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
          Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
          Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedComplianceClient final : VirtualComplianceClient {
        using ComplianceClient = C;
        Beam::GetOptionalLocalPtr<ComplianceClient> m_client;

        template<typename... Args>
        WrappedComplianceClient(Args&&... args);
        std::vector<ComplianceRuleEntry> Load(
          const Beam::ServiceLocator::DirectoryEntry& directoryEntry) override;
        ComplianceRuleId Add(
          const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
          ComplianceRuleEntry::State state,
          const ComplianceRuleSchema& schema) override;
        void Update(const ComplianceRuleEntry& entry) override;
        void Delete(ComplianceRuleId id) override;
        void Report(
          const ComplianceRuleViolationRecord& violationRecord) override;
        void MonitorComplianceRuleEntries(
          const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
          Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
          Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) override;
        void Close() override;
      };
      std::shared_ptr<VirtualComplianceClient> m_client;
  };

  template<typename T, typename... Args>
  ComplianceClientBox::ComplianceClientBox(std::in_place_type_t<T>,
    Args&&... args)
    : m_client(std::make_shared<WrappedComplianceClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename ComplianceClient>
  ComplianceClientBox::ComplianceClientBox(ComplianceClient client)
    : ComplianceClientBox(std::in_place_type<ComplianceClient>,
        std::move(client)) {}

  inline ComplianceClientBox::ComplianceClientBox(ComplianceClientBox* client)
    : ComplianceClientBox(*client) {}

  inline ComplianceClientBox::ComplianceClientBox(
    const std::shared_ptr<ComplianceClientBox>& client)
    : ComplianceClientBox(*client) {}

  inline ComplianceClientBox::ComplianceClientBox(
    const std::unique_ptr<ComplianceClientBox>& client)
    : ComplianceClientBox(*client) {}

  inline std::vector<ComplianceRuleEntry> ComplianceClientBox::Load(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    return m_client->Load(directoryEntry);
  }

  inline ComplianceRuleId ComplianceClientBox::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return m_client->Add(directoryEntry, state, schema);
  }

  inline void ComplianceClientBox::Update(const ComplianceRuleEntry& entry) {
    return m_client->Update(entry);
  }

  inline void ComplianceClientBox::Delete(ComplianceRuleId id) {
    return m_client->Delete(id);
  }

  inline void ComplianceClientBox::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    return m_client->Report(violationRecord);
  }

  inline void ComplianceClientBox::MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return m_client->MonitorComplianceRuleEntries(directoryEntry,
      std::move(queue), Beam::Store(snapshot));
  }

  inline void ComplianceClientBox::Close() {
    return m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  ComplianceClientBox::WrappedComplianceClient<C>::WrappedComplianceClient(
    Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::vector<ComplianceRuleEntry>
      ComplianceClientBox::WrappedComplianceClient<C>::Load(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    return m_client->Load(directoryEntry);
  }

  template<typename C>
  ComplianceRuleId ComplianceClientBox::WrappedComplianceClient<C>::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return m_client->Add(directoryEntry, state, schema);
  }

  template<typename C>
  void ComplianceClientBox::WrappedComplianceClient<C>::Update(
      const ComplianceRuleEntry& entry) {
    return m_client->Update(entry);
  }

  template<typename C>
  void ComplianceClientBox::WrappedComplianceClient<C>::Delete(
      ComplianceRuleId id) {
    return m_client->Delete(id);
  }

  template<typename C>
  void ComplianceClientBox::WrappedComplianceClient<C>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    return m_client->Report(violationRecord);
  }

  template<typename C>
  void ComplianceClientBox::WrappedComplianceClient<C>::
      MonitorComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return m_client->MonitorComplianceRuleEntries(directoryEntry,
      std::move(queue), Beam::Store(snapshot));
  }

  template<typename C>
  void ComplianceClientBox::WrappedComplianceClient<C>::Close() {
    return m_client->Close();
  }
}

#endif
