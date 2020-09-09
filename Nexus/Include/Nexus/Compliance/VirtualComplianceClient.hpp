#ifndef NEXUS_VIRTUAL_COMPLIANCE_CLIENT_HPP
#define NEXUS_VIRTUAL_COMPLIANCE_CLIENT_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Queues/QueueWriter.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"

namespace Nexus::Compliance {

  /** Provides a pure virtual interface to a ComplianceClient. */
  class VirtualComplianceClient : private boost::noncopyable {
    public:
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
        const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) = 0;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualComplianceClient. */
      VirtualComplianceClient() = default;
  };

  /**
   * Wraps a ComplianceClient providing it with a virtual interface.
   * @param <C> The type of ComplianceClient to wrap.
   */
  template<typename C>
  class WrapperComplianceClient : public VirtualComplianceClient {
    public:

      /** The ComplianceClient to wrap. */
      using Client = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperComplianceClient.
       * @param client The ComplianceClient to wrap.
       */
      template<typename CF>
      explicit WrapperComplianceClient(CF&& client);

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
        const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) override;

      void Close() override;

    private:
      Beam::GetOptionalLocalPtr<C> m_client;
  };

  /**
   * Wraps a ComplianceClient into a VirtualComplianceClient.
   * @param client The client to wrap.
   */
  template<typename ComplianceClient>
  std::unique_ptr<VirtualComplianceClient> MakeVirtualComplianceClient(
      ComplianceClient&& client) {
    return std::make_unique<WrapperComplianceClient<ComplianceClient>>(
      std::forward<ComplianceClient>(client));
  }

  template<typename C>
  template<typename CF>
  WrapperComplianceClient<C>::WrapperComplianceClient(CF&& client)
    : m_client(std::forward<CF>(client)) {}

  template<typename C>
  std::vector<ComplianceRuleEntry> WrapperComplianceClient<C>::Load(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    return m_client->Load(directoryEntry);
  }

  template<typename C>
  ComplianceRuleId WrapperComplianceClient<C>::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    return m_client->Add(directoryEntry, state, schema);
  }

  template<typename C>
  void WrapperComplianceClient<C>::Update(const ComplianceRuleEntry& entry) {
    return m_client->Update(entry);
  }

  template<typename C>
  void WrapperComplianceClient<C>::Delete(ComplianceRuleId id) {
    return m_client->Delete(id);
  }

  template<typename C>
  void WrapperComplianceClient<C>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    return m_client->Report(violationRecord);
  }

  template<typename C>
  void WrapperComplianceClient<C>::MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    return m_client->MonitorComplianceRuleEntries(directoryEntry, queue,
      Beam::Store(snapshot));
  }

  template<typename C>
  void WrapperComplianceClient<C>::Close() {
    return m_client->Close();
  }
}

#endif
