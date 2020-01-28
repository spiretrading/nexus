#include "Nexus/Python/Accounting.hpp"
#include <Aspen/Python/Aspen.hpp>
#include <Beam/Python/Beam.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Accounting/BookkeeperReactor.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/Position.hpp"
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  template<typename IndexType>
  void ExportKey(pybind11::object& module, const std::string& name) {
    class_<Accounting::Details::Key<IndexType>>(module, name.c_str())
      .def(init())
      .def(init<const IndexType&, CurrencyId>())
      .def(init<const Accounting::Details::Key<IndexType>&>())
      .def_readwrite("index", &Accounting::Details::Key<IndexType>::m_index)
      .def_readwrite("currency",
        &Accounting::Details::Key<IndexType>::m_currency)
      .def("__str__",
        &lexical_cast<std::string, Accounting::Details::Key<IndexType>>);
  }

  Money PythonGetTotalProfitAndLoss(
      const Portfolio<TrueAverageBookkeeper<Inventory<Position<Security>>>>&
      portfolio, CurrencyId currency) {
    return GetTotalProfitAndLoss(portfolio, currency);
  }
}

void Nexus::Python::ExportAccounting(pybind11::module& module) {
  auto submodule = module.def_submodule("accounting");
  ExportPositionOrderBook(submodule);
  ExportPosition(submodule);
  ExportSecurityInventory(submodule);
  ExportTrueAverageBookkeeper(submodule);
  ExportTrueAverageBookkeeperReactor(submodule);
  ExportTrueAveragePortfolio(submodule);
}

void Nexus::Python::ExportPositionOrderBook(pybind11::module& module) {
  auto outer = class_<PositionOrderBook>(module, "PositionOrderBook")
    .def(init())
    .def_property_readonly("live_orders", &PositionOrderBook::GetLiveOrders)
    .def_property_readonly("opening_orders",
      &PositionOrderBook::GetOpeningOrders)
    .def_property_readonly("positions", &PositionOrderBook::GetPositions)
    .def("test_opening_order_submission",
      &PositionOrderBook::TestOpeningOrderSubmission)
    .def("add", &PositionOrderBook::Add)
    .def("update", &PositionOrderBook::Update);
  class_<PositionOrderBook::PositionEntry>(outer, "Entry")
    .def(init<Security, Quantity>())
    .def_readwrite("security", &PositionOrderBook::PositionEntry::m_security)
    .def_readwrite("quantity", &PositionOrderBook::PositionEntry::m_quantity);
}

void Nexus::Python::ExportPosition(pybind11::module& module) {
  auto outer = class_<Position<Security>>(module, "Position")
    .def(init())
    .def(init<const Position<Security>::Key&>())
    .def(init<const Position<Security>&>())
    .def_readwrite("key", &Position<Security>::m_key)
    .def_readwrite("quantity", &Position<Security>::m_quantity)
    .def_readwrite("cost_basis", &Position<Security>::m_costBasis)
    .def("__str__", &lexical_cast<std::string, Position<Security>>);
  ExportKey<Security>(outer, "Key");
  module.def("average_price", &GetAveragePrice<Security>);
  module.def("side", &Accounting::GetSide<Security>);
}

void Nexus::Python::ExportSecurityInventory(pybind11::module& module) {
  using Inventory = Accounting::Inventory<Position<Security>>;
  class_<Inventory>(module, "SecurityInventory")
    .def(init())
    .def(init<const Position<Security>::Key&>())
    .def(init<const Inventory&>())
    .def_readwrite("position", &Inventory::m_position)
    .def_readwrite("gross_profit_and_loss", &Inventory::m_grossProfitAndLoss)
    .def_readwrite("fees", &Inventory::m_fees)
    .def_readwrite("volume", &Inventory::m_volume)
    .def_readwrite("transaction_count", &Inventory::m_transactionCount)
    .def("__str__", &lexical_cast<std::string, Inventory>);
}

void Nexus::Python::ExportTrueAverageBookkeeper(pybind11::module& module) {
  ExportView<std::pair<
    const TrueAverageBookkeeper<Inventory<Position<Security>>>::Key,
    TrueAverageBookkeeper<Inventory<Position<Security>>>::Inventory>>(module,
    "KeyInventoryView");
  ExportView<std::pair<const CurrencyId,
    TrueAverageBookkeeper<Inventory<Position<Security>>>::Inventory>>(module,
    "CurrencyInventoryView");
  class_<TrueAverageBookkeeper<Inventory<Position<Security>>>>(module,
      "TrueAverageBookkeeper")
    .def(init())
    .def(init<const TrueAverageBookkeeper<Inventory<Position<Security>>>&>())
    .def("record_transaction", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::RecordTransaction)
    .def("get_inventory", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetInventory)
    .def("get_total", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetTotal);
}

void Nexus::Python::ExportTrueAverageBookkeeperReactor(
    pybind11::module& module) {
  module.def("TrueAverageBookkeeperReactor",
    [] (SharedBox<const Order*> orders) {
      return to_object(BookkeeperReactor<
        TrueAverageBookkeeper<Inventory<Position<Security>>>>(
        std::move(orders)));
    });
}

void Nexus::Python::ExportTrueAveragePortfolio(pybind11::module& module) {
  using Inventory = Accounting::Inventory<Position<Security>>;
  using Portfolio = Accounting::Portfolio<TrueAverageBookkeeper<Inventory>>;
  auto outer = class_<Portfolio>(module, "TrueAveragePortfolio")
    .def(init<const MarketDatabase&>())
    .def(init<const Portfolio&>())
    .def_property_readonly("bookkeeper", &Portfolio::GetBookkeeper)
    .def_property_readonly("security_entries", &Portfolio::GetSecurityEntries)
    .def_property_readonly("unrealized_profit_and_losses",
      &Portfolio::GetUnrealizedProfitAndLosses)
    .def("update", static_cast<void (Portfolio::*)(const OrderFields&,
      const ExecutionReport& executionReport)>(&Portfolio::Update))
    .def("update_ask", &Portfolio::UpdateAsk)
    .def("update_bid", &Portfolio::UpdateBid)
    .def("update", static_cast<void (Portfolio::*)(const Security&, Money,
      Money)>(&Portfolio::Update));
  module.def("get_realized_profit_and_loss", &GetRealizedProfitAndLoss<
    Inventory::Position>);
  module.def("get_unrealized_profit_and_loss", &GetUnrealizedProfitAndLoss<
    Inventory::Position>);
  module.def("get_total_profit_and_loss", static_cast<
    boost::optional<Money> (*)(const Inventory&, const SecurityValuation&)>(
    &GetTotalProfitAndLoss<Inventory::Position>));
  module.def("get_total_profit_and_loss", &PythonGetTotalProfitAndLoss);
}
