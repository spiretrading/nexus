#ifndef NEXUS_TO_PYTHON_COMPLIANCE_RULE_DATA_STORE_HPP
#define NEXUS_TO_PYTHON_COMPLIANCE_RULE_DATA_STORE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Utilities/TypeList.hpp>
#include <boost/optional/optional.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"

namespace Nexus::Compliance {

  /**
   * Wraps a ComplianceRuleDataStore for use with Python.
   * @param <D> The type of ComplianceRuleDataStore to wrap.
   */
  template<typename D>
  class ToPythonComplianceRuleDataStore {
    public:

      /** The type of data store to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonComplianceRuleDataStore.
       * @param args The arguments to forward to the DataStore's constructor.
       */
      template<typename... Args, typename = Beam::disable_copy_constructor_t<
        ToPythonComplianceRuleDataStore, Args...>>
      ToPythonComplianceRuleDataStore(Args&&... args);

      ~ToPythonComplianceRuleDataStore();

      /** Returns the wrapped data store. */
      const DataStore& get_data_store() const;

      /** Returns the wrapped data store. */
      DataStore& get_data_store();
      std::vector<ComplianceRuleEntry> load_all_compliance_rule_entries();
      ComplianceRuleEntry::Id load_next_compliance_rule_entry_id();
      boost::optional<ComplianceRuleEntry> load_compliance_rule_entry(
        ComplianceRuleEntry::Id id);
      std::vector<ComplianceRuleEntry> load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry);
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
    ToPythonComplianceRuleDataStore<std::remove_reference_t<DataStore>>;

  template<typename D>
  template<typename... Args, typename>
  ToPythonComplianceRuleDataStore<D>::ToPythonComplianceRuleDataStore(
    Args&&... args)
    : m_data_store((Beam::Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename D>
  ToPythonComplianceRuleDataStore<D>::~ToPythonComplianceRuleDataStore() {
    auto release = Beam::Python::GilRelease();
    m_data_store.reset();
  }

  template<typename D>
  const typename ToPythonComplianceRuleDataStore<D>::DataStore&
      ToPythonComplianceRuleDataStore<D>::get_data_store() const {
    return *m_data_store;
  }

  template<typename D>
  typename ToPythonComplianceRuleDataStore<D>::DataStore&
      ToPythonComplianceRuleDataStore<D>::get_data_store() {
    return *m_data_store;
  }

  template<typename D>
  std::vector<ComplianceRuleEntry> ToPythonComplianceRuleDataStore<D>::
      load_all_compliance_rule_entries() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_all_compliance_rule_entries();
  }

  template<typename D>
  ComplianceRuleEntry::Id
      ToPythonComplianceRuleDataStore<D>::load_next_compliance_rule_entry_id() {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_next_compliance_rule_entry_id();
  }

  template<typename D>
  boost::optional<ComplianceRuleEntry> ToPythonComplianceRuleDataStore<D>::
      load_compliance_rule_entry(ComplianceRuleEntry::Id id) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_compliance_rule_entry(id);
  }

  template<typename D>
  std::vector<ComplianceRuleEntry>
      ToPythonComplianceRuleDataStore<D>::load_compliance_rule_entries(
        const Beam::ServiceLocator::DirectoryEntry& directory_entry) {
    auto release = Beam::Python::GilRelease();
    return m_data_store->load_compliance_rule_entries(directory_entry);
  }

  template<typename D>
  void ToPythonComplianceRuleDataStore<D>::store(
      const ComplianceRuleEntry& entry) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(entry);
  }

  template<typename D>
  void ToPythonComplianceRuleDataStore<D>::remove(ComplianceRuleEntry::Id id) {
    auto release = Beam::Python::GilRelease();
    m_data_store->remove(id);
  }

  template<typename D>
  void ToPythonComplianceRuleDataStore<D>::store(
      const ComplianceRuleViolationRecord& record) {
    auto release = Beam::Python::GilRelease();
    m_data_store->store(record);
  }

  template<typename D>
  void ToPythonComplianceRuleDataStore<D>::close() {
    auto release = Beam::Python::GilRelease();
    m_data_store->close();
  }
}

#endif
