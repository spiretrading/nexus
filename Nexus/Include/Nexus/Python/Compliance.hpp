#ifndef NEXUS_PYTHON_COMPLIANCE_HPP
#define NEXUS_PYTHON_COMPLIANCE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported ComplianceClient. */
  NEXUS_EXPORT_DLL pybind11::class_<ComplianceClient>&
    get_exported_compliance_client();

  /** Returns the exported ComplianceRuleDataStore. */
  NEXUS_EXPORT_DLL pybind11::class_<ComplianceRuleDataStore>&
    get_exported_compliance_rule_data_store();

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
   * Exports the application definitions.
   * @param module The module to export to.
   */
  void export_compliance_application_definitions(pybind11::module& module);

  /**
   * Exports a ComplianceClient class.
   * @param <C> The type of ComplianceClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ComplianceClient.
   */
  template<IsComplianceClient C>
  auto export_compliance_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C>(module, name.data()).
      def("load", &C::load).
      def("add", &C::add).
      def("update", &C::update).
      def("remove", &C::remove).
      def("report", &C::report).
      def("monitor_compliance_rule_entries",
        [] (C& self, const Beam::DirectoryEntry& directory_entry,
            Beam::ScopedQueueWriter<ComplianceRuleEntry> queue) {
          auto snapshot = std::vector<ComplianceRuleEntry>();
          self.monitor_compliance_rule_entries(directory_entry,
            std::move(queue), Beam::out(snapshot));
          return snapshot;
        }).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, ComplianceClient>) {
      pybind11::implicitly_convertible<C, ComplianceClient>();
      get_exported_compliance_client().
        def(pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
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
  template<IsComplianceRuleDataStore D>
  auto export_compliance_rule_data_store(
      pybind11::module& module, std::string_view name) {
    auto data_store = pybind11::class_<D>(module, name.data()).
      def("load_all_compliance_rule_entries",
        &D::load_all_compliance_rule_entries).
      def("load_next_compliance_rule_entry_id",
        &D::load_next_compliance_rule_entry_id).
      def("load_compliance_rule_entry", &D::load_compliance_rule_entry).
      def("load_compliance_rule_entries", &D::load_compliance_rule_entries).
      def("store",
        pybind11::overload_cast<const ComplianceRuleEntry&>(&D::store)).
      def("store",
        pybind11::overload_cast<const ComplianceRuleViolationRecord&>(
          &D::store)).
      def("remove", &D::remove).
      def("close", &D::close);
    if constexpr(!std::is_same_v<D, ComplianceRuleDataStore>) {
      pybind11::implicitly_convertible<D, ComplianceRuleDataStore>();
      get_exported_compliance_rule_data_store().
        def(pybind11::init<D*>(), pybind11::keep_alive<1, 2>());
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
