#ifndef NEXUS_PYTHON_COMPLIANCE_CLIENT_HPP
#define NEXUS_PYTHON_COMPLIANCE_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/Compliance/ComplianceClientBox.hpp"

namespace Nexus::Compliance {

  /**
   * Wraps a ComplianceClient for use with Python.
   * @param <C> The type of ComplianceClient to wrap.
   */
  template<typename C>
  class ToPythonComplianceClient {
    public:

      /** The type of ComplianceClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonComplianceClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args, typename =
        Beam::disable_copy_constructor_t<ToPythonComplianceClient, Args...>>
      ToPythonComplianceClient(Args&&... args);

      ~ToPythonComplianceClient();

      /** Returns the wrapped client. */
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

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
      boost::optional<Client> m_client;

      ToPythonComplianceClient(const ToPythonComplianceClient&) = delete;
      ToPythonComplianceClient& operator =(
        const ToPythonComplianceClient&) = delete;
  };

  template<typename Client>
  ToPythonComplianceClient(Client&&) ->
    ToPythonComplianceClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args, typename>
  ToPythonComplianceClient<C>::ToPythonComplianceClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonComplianceClient<C>::~ToPythonComplianceClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonComplianceClient<C>::Client&
      ToPythonComplianceClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonComplianceClient<C>::Client&
      ToPythonComplianceClient<C>::GetClient() {
    return *m_client;
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
  void ToPythonComplianceClient<C>::Update(const ComplianceRuleEntry& entry) {
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
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    auto release = Beam::Python::GilRelease();
    return m_client->MonitorComplianceRuleEntries(directoryEntry,
      std::move(queue), Beam::Store(snapshot));
  }

  template<typename C>
  void ToPythonComplianceClient<C>::Close() {
    auto release = Beam::Python::GilRelease();
    m_client->Close();
  }
}

#endif
