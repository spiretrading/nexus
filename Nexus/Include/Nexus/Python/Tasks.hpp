#ifndef NEXUS_PYTHON_TASKS_HPP
#define NEXUS_PYTHON_TASKS_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the SingleOrderTask class.
  void ExportSingleOrderTask();

  //! Exports the SingleRedisplayableOrderTask class.
  void ExportSingleRedisplayableOrderTask();

  //! Exports the Tasks namespace.
  void ExportTasks();
}
}

#endif
