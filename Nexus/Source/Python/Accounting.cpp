#include "Nexus/Python/Accounting.hpp"
#include <Aspen/Python/Aspen.hpp>
#include <Beam/Python/Beam.hpp>
#include "Nexus/Accounting/BookkeeperReactor.hpp"
#include "Nexus/Accounting/BuyingPowerModel.hpp"
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/Position.hpp"
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Accounting/ShortingModel.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_accounting(module& module) {
  auto submodule = module.def_submodule("accounting");
  export_bookkeeper<Bookkeeper>(submodule, "Bookkeeper");
  export_bookkeeper<TrueAverageBookkeeper>(submodule, "TrueAverageBookkeeper");
  export_bookkeeper_reactor(submodule);
  export_buying_power_model(submodule);
  export_inventory(submodule);
  export_portfolio<Bookkeeper>(submodule, "Portfolio");
  export_portfolio<TrueAverageBookkeeper>(submodule, "TrueAveragePortfolio");
  export_portfolio_update_entry(submodule);
  export_portfolio_controller(submodule);
  export_position(submodule);
  export_position_order_book(submodule);
  export_security_valuation(submodule);
  export_shorting_model(submodule);
}

void Nexus::Python::export_bookkeeper_reactor(module& module) {
  module.def("BookkeeperReactor",
    [] (Bookkeeper bookkeeper, SharedBox<std::shared_ptr<const Order>> orders) {
      return to_object(
        make_bookkeeper_reactor(std::move(bookkeeper), std::move(orders)));
    });
  module.def("BookkeeperReactor",
    [] (SharedBox<std::shared_ptr<const Order>> orders) {
      return to_object(
        make_bookkeeper_reactor<TrueAverageBookkeeper>(std::move(orders)));
    });
}

void Nexus::Python::export_buying_power_model(module& module) {
  class_<BuyingPowerModel>(module, "BuyingPowerModel").
    def(init()).
    def("has_order", &BuyingPowerModel::has_order, arg("id")).
    def(
      "get_buying_power", &BuyingPowerModel::get_buying_power, arg("currency")).
    def("submit", &BuyingPowerModel::submit, arg("id"), arg("fields"),
      arg("expected_price")).
    def("update", &BuyingPowerModel::update, arg("report"));
}

void Nexus::Python::export_inventory(module& module) {
  Beam::Python::ExportView<Inventory>(module, "InventoryView");
  Beam::Python::ExportView<const Inventory>(module, "InventoryConstView");
  class_<Inventory>(module, "Inventory").
    def(init()).
    def(init<const Inventory&>()).
    def(init<const Security&, CurrencyId>()).
    def(init<const Position::Key&>()).
    def(init<const Position&, Money, Money, Quantity, int>()).
    def_readwrite("position", &Inventory::m_position).
    def_readwrite("gross_profit_and_loss", &Inventory::m_gross_profit_and_loss).
    def_readwrite("fees", &Inventory::m_fees).
    def_readwrite("volume", &Inventory::m_volume).
    def_readwrite("transaction_count", &Inventory::m_transaction_count).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, Inventory>);
  module.def("is_empty", &Accounting::is_empty);
}

void Nexus::Python::export_portfolio_controller(module& module) {
  using PortfolioController =
    Accounting::PortfolioController<Portfolio<Bookkeeper>*, MarketDataClient>;
  ExportSnapshotPublisher<PortfolioUpdateEntry, Portfolio<Bookkeeper>*>(
    module, "PortfolioUpdateEntry");
  class_<PortfolioController>(module, "PortfolioController").
    def(init<Portfolio<Bookkeeper>*, MarketDataClient,
      ScopedQueueReader<std::shared_ptr<const Order>>>(), keep_alive<1, 2>()).
    def_property_readonly("publisher", &PortfolioController::get_publisher,
      return_value_policy::reference_internal);
}

void Nexus::Python::export_portfolio_update_entry(module& module) {
  class_<PortfolioUpdateEntry>(module, "PortfolioUpdateEntry").
    def(init()).
    def(init<Inventory, Money, Inventory, Money>()).
    def_readwrite(
      "security_inventory", &PortfolioUpdateEntry::m_security_inventory).
    def_readwrite(
      "unrealized_security", &PortfolioUpdateEntry::m_unrealized_security).
    def_readwrite(
      "currency_inventory", &PortfolioUpdateEntry::m_currency_inventory).
    def_readwrite(
      "unrealized_currency", &PortfolioUpdateEntry::m_unrealized_currency).
    def("__str__", &boost::lexical_cast<std::string, PortfolioUpdateEntry>);
}

void Nexus::Python::export_position(module& module) {
  auto position = class_<Position>(module, "Position").
    def(init()).
    def(init<const Position&>()).
    def(init<const Security&, CurrencyId, Quantity, Money>()).
    def_readwrite("security", &Position::m_security).
    def_readwrite("currency", &Position::m_currency).
    def_readwrite("quantity", &Position::m_quantity).
    def_readwrite("cost_basis", &Position::m_cost_basis).
    def("__str__", &boost::lexical_cast<std::string, Position>).
    def(self == self).
    def(self != self);
  class_<Position::Key>(position, "Key").
    def(init()).
    def(init<const Position::Key&>()).
    def(init<const Security&, CurrencyId>()).
    def(init([](const tuple& tuple) {
      if(tuple.size() != 2) {
        throw std::runtime_error("Invalid tuple size.");
      }
      return Position::Key(
        tuple[0].cast<Security>(), tuple[1].cast<CurrencyId>());
    })).
    def_readwrite("security", &Position::Key::m_security).
    def_readwrite("currency", &Position::Key::m_currency).
    def("__str__", &boost::lexical_cast<std::string, Position::Key>).
    def(self == self).
    def(self != self).
    def("__hash__", std::hash<Position::Key>());
  implicitly_convertible<tuple, Position::Key>();
  module.def("average_price", &Accounting::get_average_price);
  module.def("side", &Accounting::get_side);
}

void Nexus::Python::export_position_order_book(module& module) {
  auto position_order_book =
    class_<PositionOrderBook>(module, "PositionOrderBook").
      def(init()).
      def(init<View<const Inventory>>()).
      def_property_readonly("live_orders", &PositionOrderBook::get_live_orders).
      def_property_readonly("opening_orders",
        &PositionOrderBook::get_opening_orders).
      def_property_readonly("positions", &PositionOrderBook::get_positions).
      def("test_opening_order_submission",
        &PositionOrderBook::test_opening_order_submission, arg("fields")).
      def("add", &PositionOrderBook::add, arg("order")).
      def("update", &PositionOrderBook::update, arg("report"));
  class_<PositionOrderBook::PositionEntry>(
    position_order_book, "PositionEntry").
      def(init<const PositionOrderBook::PositionEntry>()).
      def(init<Security, Quantity>()).
      def_readwrite("security", &PositionOrderBook::PositionEntry::m_security).
      def_readwrite("quantity", &PositionOrderBook::PositionEntry::m_quantity).
      def(self == self).
      def(self != self).
      def("__str__",
        &boost::lexical_cast<std::string, PositionOrderBook::PositionEntry>);
}

void Nexus::Python::export_security_valuation(module& module) {
  class_<SecurityValuation>(module, "SecurityValuation").
    def(init()).
    def(init<CurrencyId>()).
    def_readwrite("currency", &SecurityValuation::m_currency).
    def_readwrite("ask_value", &SecurityValuation::m_ask_value).
    def_readwrite("bid_value", &SecurityValuation::m_bid_value).
    def("__str__", &boost::lexical_cast<std::string, SecurityValuation>);
}

void Nexus::Python::export_shorting_model(module& module) {
  class_<ShortingModel>(module, "ShortingModel").
    def(init()).
    def("submit", &ShortingModel::submit, arg("id"), arg("fields")).
    def("update", &ShortingModel::update, arg("report"));
}
