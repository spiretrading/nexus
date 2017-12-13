#include "Nexus/Python/Accounting.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Collections.hpp>
#include <Beam/Python/Copy.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/Pair.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/Vector.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
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
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace std;

namespace {
  template<typename IndexType>
  void ExportKey(const char* name) {
    class_<Accounting::Details::Key<IndexType>>(name, init<>())
      .def(init<const IndexType&, CurrencyId>())
      .def("__copy__", &MakeCopy<Accounting::Details::Key<IndexType>>)
      .def("__deepcopy__", &MakeDeepCopy<Accounting::Details::Key<IndexType>>)
      .def_readwrite("index", &Accounting::Details::Key<IndexType>::m_index)
      .def_readwrite("currency",
        &Accounting::Details::Key<IndexType>::m_currency);
  }

  Money PythonGetTotalProfitAndLoss(
      const Portfolio<TrueAverageBookkeeper<Inventory<Position<Security>>>>&
      portfolio, CurrencyId currency) {
    return GetTotalProfitAndLoss(portfolio, currency);
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
  ExportSecurityInventory();
  ExportTrueAverageBookkeeper();
  ExportTrueAveragePortfolio();
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
      ExportVector<vector<PositionOrderBook::PositionEntry>>(
        "VectorPositionEntry");
  }
}

void Nexus::Python::ExportPosition() {
  {
    scope outer =
      class_<Position<Security>>("Position", init<>())
        .def(init<const Position<Security>::Key&>())
        .def("__copy__", &MakeCopy<Position<Security>>)
        .def("__deepcopy__", &MakeDeepCopy<Position<Security>>)
        .def_readwrite("key", &Position<Security>::m_key)
        .def_readwrite("quantity", &Position<Security>::m_quantity)
        .def_readwrite("cost_basis", &Position<Security>::m_costBasis);
      ExportKey<Security>("Key");
  }
  def("average_price", &GetAveragePrice<Security>);
  def("side", &Accounting::GetSide<Security>);
}

void Nexus::Python::ExportSecurityInventory() {
  using Inventory = Accounting::Inventory<Position<Security>>;
  class_<Inventory>("SecurityInventory", init<>())
    .def(init<const Position<Security>::Key&>())
    .def("__copy__", &MakeCopy<Inventory>)
    .def("__deepcopy__", &MakeDeepCopy<Inventory>)
    .def_readwrite("position", &Inventory::m_position)
    .def_readwrite("gross_profit_and_loss", &Inventory::m_grossProfitAndLoss)
    .def_readwrite("fees", &Inventory::m_fees)
    .def_readwrite("volume", &Inventory::m_volume)
    .def_readwrite("transaction_count", &Inventory::m_transactionCount);
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
    .def("__copy__", &MakeCopy<TrueAverageBookkeeper<
      Inventory<Position<Security>>>>)
    .def("__deepcopy__", &MakeDeepCopy<TrueAverageBookkeeper<
      Inventory<Position<Security>>>>)
    .def("record_transaction", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::RecordTransaction)
    .def("get_inventory", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetInventory,
      return_value_policy<copy_const_reference>())
    .def("get_total", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetTotal,
      return_value_policy<copy_const_reference>());
}

void Nexus::Python::ExportTrueAveragePortfolio() {
  using Inventory = Accounting::Inventory<Position<Security>>;
  using Portfolio = Accounting::Portfolio<TrueAverageBookkeeper<Inventory>>;
  {
    scope outer =
      class_<Portfolio>("TrueAveragePortfolio", init<const MarketDatabase&>())
        .def("__copy__", &MakeCopy<Portfolio>)
        .def("__deepcopy__", &MakeDeepCopy<Portfolio>)
        .add_property("bookkeeper", make_function(&Portfolio::GetBookkeeper,
        return_value_policy<copy_const_reference>()))
        .add_property("security_entries", make_function(
          &Portfolio::GetSecurityEntries,
          return_value_policy<copy_const_reference>()))
        .add_property("unrealized_profit_and_losses", make_function(
          &Portfolio::GetUnrealizedProfitAndLosses,
          return_value_policy<copy_const_reference>()))
        .def("update", static_cast<void (Portfolio::*)(const OrderFields&,
          const ExecutionReport& executionReport)>(&Portfolio::Update))
        .def("update_ask", &Portfolio::UpdateAsk)
        .def("update_bid", &Portfolio::UpdateBid)
        .def("update", static_cast<void (Portfolio::*)(const Security&, Money,
          Money)>(&Portfolio::Update));
  }
  def("get_realized_profit_and_loss", &GetRealizedProfitAndLoss<
    Inventory::Position>);
  def("get_unrealized_profit_and_loss", &GetUnrealizedProfitAndLoss<
    Inventory::Position>);
  def("get_total_profit_and_loss", static_cast<
    boost::optional<Money> (*)(const Inventory&, const SecurityValuation&)>(
    &GetTotalProfitAndLoss<Inventory::Position>));
  def("get_total_profit_and_loss", &PythonGetTotalProfitAndLoss);
}
