#ifndef NEXUS_PYTHON_COMPLIANCE_CLIENT_HPP
#define NEXUS_PYTHON_COMPLIANCE_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Nexus/Compliance/ComplianceClient.hpp"

namespace Nexus {

  /**
   * Wraps a ComplianceClient for use with Python.
   * @param <C> The type of ComplianceClient to wrap.
   */
  template<IsComplianceClient C>
  class ToPythonComplianceClient {
    public:

      /** The type of ComplianceClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonComplianceClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonComplianceClient(Args&&... args);

      ~ToPythonComplianceClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      std::vector<ComplianceRuleEntry> load(
        const Beam::DirectoryEntry& directory_entry);
      ComplianceRuleEntry::Id add(
        const Beam::DirectoryEntry& directory_entry,
        ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema);
      void update(const ComplianceRuleEntry& entry);
      void remove(ComplianceRuleEntry::Id id);
      void report(const ComplianceRuleViolationRecord& record);
      void monitor_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry,
        Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
        Beam::Out<std::vector<ComplianceRuleEntry>> snapshot);
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonComplianceClient(const ToPythonComplianceClient&) = delete;
      ToPythonComplianceClient& operator =(
        const ToPythonComplianceClient&) = delete;
  };

  template<typename Client>
  ToPythonComplianceClient(Client&&) ->
    ToPythonComplianceClient<std::remove_cvref_t<Client>>;

  template<IsComplianceClient C>
  template<typename... Args>
  ToPythonComplianceClient<C>::ToPythonComplianceClient(Args&&... args)
    : m_client((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsComplianceClient C>
  ToPythonComplianceClient<C>::~ToPythonComplianceClient() {
    auto release = Beam::Python::GilRelease();
    m_client.reset();
  }

  template<IsComplianceClient C>
  typename ToPythonComplianceClient<C>::Client&
      ToPythonComplianceClient<C>::get() {
    return *m_client;
  }

  template<IsComplianceClient C>
  const typename ToPythonComplianceClient<C>::Client&
      ToPythonComplianceClient<C>::get() const {
    return *m_client;
  }

  template<IsComplianceClient C>
  std::vector<ComplianceRuleEntry> ToPythonComplianceClient<C>::load(
      const Beam::DirectoryEntry& directory_entry) {
    auto release = Beam::Python::GilRelease();
    return m_client->load(directory_entry);
  }

  template<IsComplianceClient C>
  ComplianceRuleEntry::Id ToPythonComplianceClient<C>::add(
      const Beam::DirectoryEntry& directory_entry,
      ComplianceRuleEntry::State state, const ComplianceRuleSchema& schema) {
    auto release = Beam::Python::GilRelease();
    return m_client->add(directory_entry, state, schema);
  }

  template<IsComplianceClient C>
  void ToPythonComplianceClient<C>::update(const ComplianceRuleEntry& entry) {
    auto release = Beam::Python::GilRelease();
    m_client->update(entry);
  }

  template<IsComplianceClient C>
  void ToPythonComplianceClient<C>::remove(ComplianceRuleEntry::Id id) {
    auto release = Beam::Python::GilRelease();
    m_client->remove(id);
  }

  template<IsComplianceClient C>
  void ToPythonComplianceClient<C>::report(
      const ComplianceRuleViolationRecord& record) {
    auto release = Beam::Python::GilRelease();
    m_client->report(record);
  }

  template<IsComplianceClient C>
  void ToPythonComplianceClient<C>::monitor_compliance_rule_entries(
      const Beam::DirectoryEntry& directory_entry,
      Beam::ScopedQueueWriter<ComplianceRuleEntry> queue,
      Beam::Out<std::vector<ComplianceRuleEntry>> snapshot) {
    auto release = Beam::Python::GilRelease();
    m_client->monitor_compliance_rule_entries(directory_entry, std::move(queue),
      Beam::out(snapshot));
  }

  template<IsComplianceClient C>
  void ToPythonComplianceClient<C>::close() {
    auto release = Beam::Python::GilRelease();
    m_client->close();
  }
}

#endif
