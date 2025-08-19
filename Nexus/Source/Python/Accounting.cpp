#include "Nexus/Python/Accounting.hpp"
#include <Aspen/Python/Aspen.hpp>
#include <Beam/Python/Beam.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Accounting/BookkeeperReactor.hpp"
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/Position.hpp"
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Accounting/ShortingModel.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
#if 0
  Money PythonGetTotalProfitAndLoss(
      const Portfolio<TrueAverageBookkeeper<Inventory<Position<Security>>>>&
      portfolio, CurrencyId currency) {
    return GetTotalProfitAndLoss(portfolio, currency);
  }
#endif
}

void Nexus::Python::export_accounting(module& module) {
  auto submodule = module.def_submodule("accounting");
  export_position<object>(submodule, "Position");
  export_position<Security>(submodule, "SecurityPosition");
  export_inventory<Position<object>>(submodule, "Inventory");
  export_inventory<Position<Security>>(submodule, "SecurityInventory");
  export_bookkeeper<Bookkeeper<Inventory<Position<object>>>>(
    submodule, "Bookkeeper");
  export_bookkeeper<Bookkeeper<Inventory<Position<Security>>>>(
    submodule, "SecurityBookkeeper");
  export_bookkeeper<TrueAverageBookkeeper<Inventory<Position<object>>>>(
    submodule, "TrueAverageBookkeeper");
  export_bookkeeper<TrueAverageBookkeeper<Inventory<Position<Security>>>>(
    submodule, "SecurityTrueAverageBookkeeper");
  export_buying_power_model(submodule);
  export_shorting_model(submodule);
/*
  ExportPortfolioUpdateEntry(submodule);
  ExportPositionOrderBook(submodule);
  ExportSecurityValuation(submodule);
  ExportTrueAverageBookkeeperReactor(submodule);
  ExportTrueAveragePortfolio(submodule);
*/
}

void Nexus::Python::export_buying_power_model(pybind11::module& module) {
  class_<BuyingPowerModel>(module, "BuyingPowerModel").
    def(init()).
    def("has_order", &BuyingPowerModel::has_order, arg("id")).
    def(
      "get_buying_power", &BuyingPowerModel::get_buying_power, arg("currency")).
    def("submit", &BuyingPowerModel::submit, arg("id"), arg("fields"),
      arg("expected_price")).
    def("update", &BuyingPowerModel::update, arg("report"));
}

void Nexus::Python::export_shorting_model(pybind11::module& module) {
  class_<ShortingModel>(module, "ShortingModel").
    def(init()).
    def("submit", &ShortingModel::submit, arg("id"), arg("fields")).
    def("update", &ShortingModel::update, arg("report"));
}

/*
void Nexus::Python::ExportPortfolioUpdateEntry(module& module) {
  using Entry = PortfolioUpdateEntry<Inventory<Position<Security>>>;
  class_<Entry>(module, "PortfolioUpdateEntry").
    def_readwrite("security_inventory", &Entry::m_securityInventory).
    def_readwrite("unrealized_security", &Entry::m_unrealizedSecurity).
    def_readwrite("currency_inventory", &Entry::m_currencyInventory).
    def_readwrite("unrealized_currency", &Entry::m_unrealizedCurrency);
}

void Nexus::Python::ExportPositionOrderBook(module& module) {
  auto outer = class_<PositionOrderBook>(module, "PositionOrderBook").
    def(init()).
    def_property_readonly("live_orders", &PositionOrderBook::GetLiveOrders).
    def_property_readonly("opening_orders",
      &PositionOrderBook::GetOpeningOrders).
    def_property_readonly("positions", &PositionOrderBook::GetPositions).
    def("test_opening_order_submission",
      &PositionOrderBook::TestOpeningOrderSubmission).
    def("add", &PositionOrderBook::Add).
    def("update", &PositionOrderBook::Update);
  class_<PositionOrderBook::PositionEntry>(outer, "Entry").
    def(init<Security, Quantity>()).
    def_readwrite("security", &PositionOrderBook::PositionEntry::m_security).
    def_readwrite("quantity", &PositionOrderBook::PositionEntry::m_quantity);
}

void Nexus::Python::ExportSecurityValuation(module& module) {
  class_<SecurityValuation>(module, "SecurityValuation").
    def(init()).
    def(init<const SecurityValuation&>()).
    def(init<CurrencyId>()).
    def_readwrite("currency", &SecurityValuation::m_currency).
    def_readwrite("ask_value", &SecurityValuation::m_askValue).
    def_readwrite("bid_value", &SecurityValuation::m_bidValue);
}

void Nexus::Python::ExportTrueAverageBookkeeperReactor(module& module) {
  module.def("TrueAverageBookkeeperReactor",
    [] (SharedBox<const Order*> orders) {
      return to_object(BookkeeperReactor<
        TrueAverageBookkeeper<Inventory<Position<Security>>>>(
        std::move(orders)));
    });
}

void Nexus::Python::ExportTrueAveragePortfolio(module& module) {
  using Inventory = Accounting::Inventory<Position<Security>>;
  using Portfolio = Accounting::Portfolio<TrueAverageBookkeeper<Inventory>>;
  auto outer = class_<Portfolio>(module, "TrueAveragePortfolio").
    def(init<const MarketDatabase&>()).
    def(init<const MarketDatabase&, TrueAverageBookkeeper<Inventory>>()).
    def(init<const Portfolio&>()).
    def_property_readonly("bookkeeper", &Portfolio::GetBookkeeper).
    def_property_readonly("security_entries", &Portfolio::GetSecurityEntries).
    def_property_readonly("unrealized_profit_and_losses",
      &Portfolio::GetUnrealizedProfitAndLosses).
    def("update", static_cast<bool (Portfolio::*)(const OrderFields&,
      const ExecutionReport& executionReport)>(&Portfolio::Update)).
    def("update_ask", &Portfolio::UpdateAsk).
    def("update_bid", &Portfolio::UpdateBid).
    def("update", static_cast<bool (Portfolio::*)(
      const Security&, Money, Money)>(&Portfolio::Update));
  class_<Portfolio::SecurityEntry>(outer, "SecurityEntry").
    def(init<CurrencyId>()).
    def(init<const Portfolio::SecurityEntry&>()).
    def_readwrite("valuation", &Portfolio::SecurityEntry::m_valuation).
    def_readwrite("unrealized", &Portfolio::SecurityEntry::m_unrealized);
  module.def("get_realized_profit_and_loss",
    &GetRealizedProfitAndLoss<Inventory::Position>);
  module.def("get_unrealized_profit_and_loss",
    &GetUnrealizedProfitAndLoss<Inventory::Position>);
  module.def("get_total_profit_and_loss", static_cast<
    boost::optional<Money> (*)(const Inventory&, const SecurityValuation&)>(
      &GetTotalProfitAndLoss<Inventory::Position>));
  module.def("get_total_profit_and_loss", &PythonGetTotalProfitAndLoss);
}
*/
