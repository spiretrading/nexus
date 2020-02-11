#ifndef NEXUS_PYTHON_ORDER_EXECUTION_SERVICE_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the ApplicationOrderExecutionClient class.
   * @param module The module to export to.
   */
  void ExportApplicationOrderExecutionClient(pybind11::module& module);

  /**
   * Exports the ExecutionReport struct.
   * @param module The module to export to.
   */
  void ExportExecutionReport(pybind11::module& module);

  /**
   * Exports the MockOrderExecutionDriver class.
   * @param module The module to export to.
   */
  void ExportMockOrderExecutionDriver(pybind11::module& module);

  /**
   * Exports the Order class.
   * @param module The module to export to.
   */
  void ExportOrder(pybind11::module& module);

  /**
   * Exports the OrderCancellationReactor class.
   * @param module The module to export to.
   */
  void ExportOrderCancellationReactor(pybind11::module& module);

  /**
   * Exports the OrderExecutionClient class.
   * @param module The module to export to.
   */
  void ExportOrderExecutionClient(pybind11::module& module);

  /**
   * Exports the OrderExecutionService namespace.
   * @param module The module to export to.
   */
  void ExportOrderExecutionService(pybind11::module& module);

  /**
   * Exports the OrderExecutionServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportOrderExecutionServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the OrderFields struct.
   * @param module The module to export to.
   */
  void ExportOrderFields(pybind11::module& module);

  /**
   * Exports the OrderInfo struct.
   * @param module The module to export to.
   */
  void ExportOrderInfo(pybind11::module& module);

  /**
   * Exports the OrderReactor class.
   * @param module The module to export to.
   */
  void ExportOrderReactor(pybind11::module& module);

  /**
   * Exports the OrderRecord struct.
   * @param module The module to export to.
   */
  void ExportOrderRecord(pybind11::module& module);

  /**
   * Exports the PrimitiveOrder class.
   * @param module The module to export to.
   */
  void ExportPrimitiveOrder(pybind11::module& module);

  /**
   * Exports the standard queries.
   * @param module The module to export to.
   */
  void ExportStandardQueries(pybind11::module& module);
}

#endif
