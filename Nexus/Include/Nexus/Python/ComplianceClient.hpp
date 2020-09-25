#ifndef NEXUS_PYTHON_COMPLIANCE_CLIENT_HPP
#define NEXUS_PYTHON_COMPLIANCE_CLIENT_HPP
#include <Beam/Python/GilRelease.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/Compliance/VirtualComplianceClient.hpp"

namespace Nexus::Compliance {

  /**
   * Wraps a ComplianceClient for use with Python.
   * @param <C> The type of ComplianceClient to wrap.
   */
  template<typename C>
  class ToPythonComplianceClient final : public VirtualComplianceClient {
    public:

      /** The type of ComplianceClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonComplianceClient.
       * @param client The ComplianceClient to wrap.
       */
      ToPythonComplianceClient(std::unique_ptr<Client> client);

      ~ToPythonComplianceClient() override;

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
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonComplianceClient.
   * @param client The ComplianceClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonComplianceClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonComplianceClient<Client>>(
      std::move(client));
  }

  template<typename C>
  ToPythonComplianceClient<C>::ToPythonComplianceClient(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonComplianceClient<C>::~ToPythonComplianceClient() {
    Close();
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  std::vector<ComplianceRuleEntry> ToPythonComplianceClient<C>::Load(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    auto release = Beam::Python::GilRelease();
    return m_client->Load(directoryEntry);
  }

  template<typename C>
  ComplianceRuleId ToPythonComplianceClient<C>::Add(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto release = Beam::Python::GilRelease();
    return m_client->Add(directoryEntry, state, schema);
  }

  template<typename C>
  void ToPythonComplianceClient<C>::Update(
      const ComplianceRuleEntry& entry) {
    auto release = Beam::Python::GilRelease();
    return m_client->Update(entry);
  }

  template<typename C>
  void ToPythonComplianceClient<C>::Delete(ComplianceRuleId id) {
    auto release = Beam::Python::GilRelease();
    return m_client->Delete(id);
  }

  template<typename C>
  void ToPythonComplianceClient<C>::Report(
      const ComplianceRuleViolationRecord& violationRecord) {
    auto release = Beam::Python::GilRelease();
    return m_client->Report(violationRecord);
  }

  template<typename C>
  void ToPythonComplianceClient<C>::MonitorComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
      const std::shared_ptr<Beam::QueueWriter<ComplianceRuleEntry>>& queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    auto release = Beam::Python::GilRelease();
    return m_client->MonitorComplianceRuleEntries(directoryEntry, queue,
      snapshot);
  }

  template<typename C>
  void ToPythonComplianceClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
