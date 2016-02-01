#include "Nexus/Python/Accounting.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Collections.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/Pair.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include "Nexus/Accounting/Position.hpp"
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::Python;
using namespace std;

namespace {
  template<typename IndexType>
  void ExportKey(const char* name) {
    class_<Accounting::Details::Key<IndexType>>(name, init<>())
      .def(init<const IndexType&, CurrencyId>())
      .def_readwrite("index", &Accounting::Details::Key<IndexType>::m_index)
      .def_readwrite("currency",
        &Accounting::Details::Key<IndexType>::m_currency);
  }
}

void Nexus::Python::ExportAccounting() {
  string nestedName = extract<string>(scope().attr("__name__") + ".accounting");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("accounting") = nestedModule;
  scope parent = nestedModule;
  ExportPositionOrderBook();
  ExportPosition();
  ExportTrueAverageBookkeeper();
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

void Nexus::Python::ExportPosition() {
  {
    scope outer =
      class_<Position<Security>>("Position", init<>())
        .def(init<const Position<Security>::Key&>())
        .def_readwrite("key", &Position<Security>::m_key)
        .def_readwrite("quantity", &Position<Security>::m_quantity)
        .def_readwrite("cost_basis", &Position<Security>::m_costBasis);
      ExportKey<Security>("Key");
  }
  def("average_price", &GetAveragePrice<Security>);
  def("side", &Accounting::GetSide<Security>);
}

void Nexus::Python::ExportTrueAverageBookkeeper() {
  ExportPair<const TrueAverageBookkeeper<Inventory<Position<Security>>>::Key,
    TrueAverageBookkeeper<Inventory<Position<Security>>>::Inventory>();
  ExportPair<const CurrencyId,
    TrueAverageBookkeeper<Inventory<Position<Security>>>::Inventory>();
  ExportView<std::pair<
    const TrueAverageBookkeeper<Inventory<Position<Security>>>::Key,
    TrueAverageBookkeeper<Inventory<Position<Security>>>::Inventory>>(
    "KeyInventoryView");
  ExportView<std::pair<const CurrencyId,
    TrueAverageBookkeeper<Inventory<Position<Security>>>::Inventory>>(
    "CurrencyInventoryView");
  class_<TrueAverageBookkeeper<Inventory<Position<Security>>>>(
      "TrueAverageBookkeeper", init<>())
    .def("record_transaction", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::RecordTransaction)
    .def("get_inventory", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetInventory,
      return_value_policy<copy_const_reference>())
    .def("get_total", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetTotal,
      return_value_policy<copy_const_reference>());
}
