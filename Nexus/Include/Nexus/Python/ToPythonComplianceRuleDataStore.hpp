#ifndef NEXUS_TO_PYTHON_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_TO_PYTHON_COMPLIANCE_RULE_DATA_STORE_HPP
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"

namespace Nexus {

  /**
   * Wraps a ComplianceRuleDataStore for use with Python.
   * @param <D> The type of ComplianceRuleDataStore to wrap.
   */
  template<IsComplianceRuleDataStore D>
  class ToPythonComplianceRuleDataStore {
    public:

      /** The type of data store to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonComplianceRuleDataStore in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonComplianceRuleDataStore(Args&&... args);

      ~ToPythonComplianceRuleDataStore();

      /** Returns a reference to the underlying data store. */
      DataStore& get();

      /** Returns a reference to the underlying data store. */
      const DataStore& get() const;

      /** Returns a reference to the underlying data store. */
      DataStore& operator *();

      /** Returns a reference to the underlying data store. */
      const DataStore& operator *() const;

      /** Returns a pointer to the underlying data store. */
      DataStore* operator ->();

      /** Returns a pointer to the underlying data store. */
      const DataStore* operator ->() const;

      std::vector<ComplianceRuleEntry> load_all_compliance_rule_entries();
      ComplianceRuleEntry::Id load_next_compliance_rule_entry_id();
      boost::optional<ComplianceRuleEntry> load_compliance_rule_entry(
        ComplianceRuleEntry::Id id);
      std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry);
      void store(const ComplianceRuleEntry& entry);
      void remove(ComplianceRuleEntry::Id id);
      void store(const ComplianceRuleViolationRecord& record);
      void close();

    private:
      boost::optional<DataStore> m_data_store;

      ToPythonComplianceRuleDataStore(
        const ToPythonComplianceRuleDataStore&) = delete;
      ToPythonComplianceRuleDataStore& operator =(
        const ToPythonComplianceRuleDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonComplianceRuleDataStore(DataStore&&) ->
    ToPythonComplianceRuleDataStore<std::remove_cvref_t<DataStore>>;

  template<IsComplianceRuleDataStore D>
  template<typename... Args>
  ToPythonComplianceRuleDataStore<D>::ToPythonComplianceRuleDataStore(
    Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsComplianceRuleDataStore D>
  ToPythonComplianceRuleDataStore<D>::~ToPythonComplianceRuleDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<IsComplianceRuleDataStore D>
  typename ToPythonComplianceRuleDataStore<D>::DataStore&
      ToPythonComplianceRuleDataStore<D>::get() {
    return *m_data_store;
  }

  template<IsComplianceRuleDataStore D>
  const typename ToPythonComplianceRuleDataStore<D>::DataStore&
      ToPythonComplianceRuleDataStore<D>::get() const {
    return *m_data_store;
  }

  template<IsComplianceRuleDataStore D>
  typename ToPythonComplianceRuleDataStore<D>::DataStore&
      ToPythonComplianceRuleDataStore<D>::operator *() {
    return *m_data_store;
  }

  template<IsComplianceRuleDataStore D>
  const typename ToPythonComplianceRuleDataStore<D>::DataStore&
      ToPythonComplianceRuleDataStore<D>::operator *() const {
    return *m_data_store;
  }

  template<IsComplianceRuleDataStore D>
  typename ToPythonComplianceRuleDataStore<D>::DataStore*
      ToPythonComplianceRuleDataStore<D>::operator ->() {
    return m_data_store.get_ptr();
  }

  template<IsComplianceRuleDataStore D>
  const typename ToPythonComplianceRuleDataStore<D>::DataStore*
      ToPythonComplianceRuleDataStore<D>::operator ->() const {
    return m_data_store.get_ptr();
  }

  template<IsComplianceRuleDataStore D>
  std::vector<ComplianceRuleEntry> ToPythonComplianceRuleDataStore<D>::
      load_all_compliance_rule_entries() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_compliance_rule_entries();
  }

  template<IsComplianceRuleDataStore D>
  ComplianceRuleEntry::Id
      ToPythonComplianceRuleDataStore<D>::load_next_compliance_rule_entry_id() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_next_compliance_rule_entry_id();
  }

  template<IsComplianceRuleDataStore D>
  boost::optional<ComplianceRuleEntry> ToPythonComplianceRuleDataStore<D>::
      load_compliance_rule_entry(ComplianceRuleEntry::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_compliance_rule_entry(id);
  }

  template<IsComplianceRuleDataStore D>
  std::vector<ComplianceRuleEntry>
      ToPythonComplianceRuleDataStore<D>::load_compliance_rule_entries(
        const Beam::DirectoryEntry& directory_entry) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  template<IsComplianceRuleDataStore D>
  void ToPythonComplianceRuleDataStore<D>::store(
      const ComplianceRuleEntry& entry) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(entry);
  }

  template<IsComplianceRuleDataStore D>
  void ToPythonComplianceRuleDataStore<D>::remove(ComplianceRuleEntry::Id id) {
    auto release = Beam::Python::GilRelease();
    m_data_store->remove(id);
  }

  template<IsComplianceRuleDataStore D>
  void ToPythonComplianceRuleDataStore<D>::store(
      const ComplianceRuleViolationRecord& record) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(record);
  }

  template<IsComplianceRuleDataStore D>
  void ToPythonComplianceRuleDataStore<D>::close() {
    auto release = Beam::Python::GilRelease();
    m_data_store->close();
  }
}

#endif
