#ifndef NEXUS_PYTHONQUERIES_HPP
#define NEXUS_PYTHONQUERIES_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the DataType classes.
  void ExportDataType();

  //! Exports the Queries namespace.
  void ExportQueries();

  //! Exports the Value classes.
  void ExportValue();
}
}

#endif
