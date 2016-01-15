#include "Nexus/Python/Accounting.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include "Nexus/Accounting/PositionOrderBook.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::Python;
using namespace std;

void Nexus::Python::ExportAccounting() {
  string nestedName = extract<string>(scope().attr("__name__") + ".accounting");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("accounting") = nestedModule;
  scope parent = nestedModule;
  ExportPositionOrderBook();
}

void Nexus::Python::ExportPositionOrderBook() {
  {
    scope outer =
      class_<PositionOrderBook, noncopyable>("PositionOrderBook", init<>())
        .add_property("live_orders",
          make_function(&PositionOrderBook::GetLiveOrders,
          return_value_policy<return_by_value>()))
        .add_property("opening_orders",
          make_function(&PositionOrderBook::GetOpeningOrders,
          return_value_policy<return_by_value>()))
        .add_property("positions",
          make_function(&PositionOrderBook::GetPositions,
          return_value_policy<return_by_value>()))
        .def("test_opening_order_submission",
          &PositionOrderBook::TestOpeningOrderSubmission)
        .def("add", &PositionOrderBook::Add)
        .def("update", &PositionOrderBook::Update);
      class_<PositionOrderBook::PositionEntry>("Entry",
        init<Security, Quantity>())
        .def_readwrite("security",
          &PositionOrderBook::PositionEntry::m_security)
        .def_readwrite("quantity",
          &PositionOrderBook::PositionEntry::m_quantity);
      class_<vector<PositionOrderBook::PositionEntry>>("VectorPositionEntry")
        .def(vector_indexing_suite<vector<PositionOrderBook::PositionEntry>>());
  }
}
