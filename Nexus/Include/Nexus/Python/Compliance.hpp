#ifndef NEXUS_PYTHON_COMPLIANCE_HPP
#define NEXUS_PYTHON_COMPLIANCE_HPP
#include <string_view>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"

namespace Nexus::Python {

  /**
   * Exports the CachedComplianceRuleDataStore class.
   * @param module The module to export to.
   */
  void export_cached_compliance_rule_data_store(pybind11::module& module);

  /**
   * Exports the Compliance namespace.
   * @param module The module to export to.
   */
  void export_compliance(pybind11::module& module);

  /**
   * Exports a ComplianceClient class.
   * @param <C> The type of ComplianceClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ComplianceClient.
   */
  template<typename C>
  auto export_compliance_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C, std::shared_ptr<C>>(module, name.data()).
      def("load", &C::load).
      def("add", &C::add).
      def("update", &C::update).
      def("remove", &C::remove).
      def("report", &C::report).
      def("monitor_compliance_rule_entries", [] (C& self,
          const Beam::ServiceLocator::DirectoryEntry& directory_entry,
          Beam::ScopedQueueWriter<Compliance::ComplianceRuleEntry> queue) {
        auto snapshot = std::vector<Compliance::ComplianceRuleEntry>();
        self.monitor_compliance_rule_entries(directory_entry,
          std::move(queue), Beam::Store(snapshot));
        return snapshot;
      }).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, Compliance::ComplianceClient>) {
      pybind11::implicitly_convertible<C, Compliance::ComplianceClient>();
    }
    return client;
  }

  /**
   * Exports the ComplianceParameter struct.
   * @param module The module to export to.
   */
  void export_compliance_parameter(pybind11::module& module);

  /**
   * Exports the ComplianceRuleDataStore class.
   * @param <D> The type of ComplianceDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ComplianceRuleDataStore.
   */
  template<typename D>
  auto export_compliance_rule_data_store(
      pybind11::module& module, std::string_view name) {
    auto data_store = pybind11::class_<D, std::shared_ptr<D>>(
      module, name.data()).
        def("load_all_compliance_rule_entries",
          &D::load_all_compliance_rule_entries).
        def("load_next_compliance_rule_entry_id",
          &D::load_next_compliance_rule_entry_id).
        def("load_compliance_rule_entry",
          &D::load_compliance_rule_entry).
        def("load_compliance_rule_entries",
          &D::load_compliance_rule_entries).
        def("store", static_cast<void (D::*)(
          const Compliance::ComplianceRuleEntry&)>(&D::store)).
        def("store", static_cast<void (D::*)(
          const Compliance::ComplianceRuleViolationRecord&)>(&D::store)).
        def("remove", &D::remove).
        def("close", &D::close);
    if constexpr(!std::is_same_v<D, Compliance::ComplianceRuleDataStore>) {
      pybind11::implicitly_convertible<
        D, Compliance::ComplianceRuleDataStore>();
    }
    return data_store;
  }

  /**
   * Exports the ComplianceRuleDataStoreException class.
   * @param module The module to export to.
   */
  void export_compliance_rule_data_store_exception(pybind11::module& module);

  /**
   * Exports the ComplianceRuleEntry class.
   * @param module The module to export to.
   */
  void export_compliance_rule_entry(pybind11::module& module);

  /**
   * Exports the ComplianceRuleSchema class.
   * @param module The module to export to.
   */
  void export_compliance_rule_schema(pybind11::module& module);

  /**
   * Exports the ComplianceRuleViolationRecord struct.
   * @param module The module to export to.
   */
  void export_compliance_rule_violation_record(pybind11::module& module);

  /**
   * Exports the ComplianceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_compliance_test_environment(pybind11::module& module);

  /**
   * Exports the LocalComplianceRuleDataStore class.
   * @param module The module to export to.
   */
  void export_local_compliance_rule_data_store(pybind11::module& module);

  /**
   * Exports the MySqlComplianceRuleDataStore class.
   * @param module The module to export to.
   */
  void export_mysql_compliance_rule_data_store(pybind11::module& module);

  /**
   * Exports the SqliteComplianceRuleDataStore class.
   * @param module The module to export to.
   */
  void export_sqlite_compliance_rule_data_store(pybind11::module& module);
}

#endif
