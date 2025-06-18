#include "Nexus/Python/Accounting.hpp"
#include <Aspen/Python/Aspen.hpp>
#include <Beam/Python/Beam.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Accounting/BookkeeperReactor.hpp"
#include "Nexus/Accounting/BuyingPowerModel.hpp"
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
  void ExportKey(object& module, const std::string& name) {
    class_<Accounting::Details::Key<IndexType>>(module, name.c_str()).
      def(init()).
      def(init<IndexType, CurrencyId>()).
      def(init<const Accounting::Details::Key<IndexType>&>()).
      def_readwrite("index", &Accounting::Details::Key<IndexType>::m_index).
      def_readwrite("currency",
        &Accounting::Details::Key<IndexType>::m_currency).
      def("__str__",
        &lexical_cast<std::string, Accounting::Details::Key<IndexType>>).
      def(self == self).
      def(self != self);
  }

  Money PythonGetTotalProfitAndLoss(
      const Portfolio<TrueAverageBookkeeper<Inventory<Position<Security>>>>&
      portfolio, CurrencyId currency) {
    return GetTotalProfitAndLoss(portfolio, currency);
  }
}

void Nexus::Python::ExportBuyingPowerModel(module& module) {
  auto outer = class_<BuyingPowerModel>(module, "BuyingPowerModel").
    def(init()).
    def("has_order", &BuyingPowerModel::HasOrder).
    def("get_buying_power", &BuyingPowerModel::GetBuyingPower).
    def("submit", &BuyingPowerModel::Submit).
    def("update", &BuyingPowerModel::Update);
}

void Nexus::Python::ExportAccounting(module& module) {
  auto submodule = module.def_submodule("accounting");
  ExportBuyingPowerModel(submodule);
  ExportPortfolioUpdateEntry(submodule);
  ExportPositionOrderBook(submodule);
  ExportPosition(submodule);
  ExportSecurityInventory(submodule);
  ExportSecurityValuation(submodule);
  ExportTrueAverageBookkeeper(submodule);
  ExportTrueAverageBookkeeperReactor(submodule);
  ExportTrueAveragePortfolio(submodule);
}

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

void Nexus::Python::ExportPosition(module& module) {
  auto outer = class_<Position<Security>>(module, "Position").
    def(init()).
    def(init<Position<Security>::Key>()).
    def(init<const Position<Security>&>()).
    def(init<Position<Security>::Key, Quantity, Money>()).
    def_readwrite("key", &Position<Security>::m_key).
    def_readwrite("quantity", &Position<Security>::m_quantity).
    def_readwrite("cost_basis", &Position<Security>::m_costBasis).
    def("__str__", &lexical_cast<std::string, Position<Security>>).
    def(self == self).
    def(self != self);
  ExportKey<Security>(outer, "Key");
  module.def("average_price", &GetAveragePrice<Security>);
  module.def("side", &Accounting::GetSide<Security>);
}

void Nexus::Python::ExportSecurityInventory(module& module) {
  using Inventory = Accounting::Inventory<Position<Security>>;
  class_<Inventory>(module, "SecurityInventory").
    def(init()).
    def(init<Position<Security>::Key>()).
    def(init<Position<Security>, Money, Money, Quantity, int>()).
    def(init<const Inventory&>()).
    def_readwrite("position", &Inventory::m_position).
    def_readwrite("gross_profit_and_loss", &Inventory::m_grossProfitAndLoss).
    def_readwrite("fees", &Inventory::m_fees).
    def_readwrite("volume", &Inventory::m_volume).
    def_readwrite("transaction_count", &Inventory::m_transactionCount).
    def("__str__", &lexical_cast<std::string, Inventory>).
    def(self == self).
    def(self != self);
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

void Nexus::Python::ExportTrueAverageBookkeeper(module& module) {
  ExportView<TrueAverageBookkeeper<Inventory<Position<Security>>>::Inventory>(
    module, "InventoryView");
  class_<TrueAverageBookkeeper<Inventory<Position<Security>>>>(module,
      "TrueAverageBookkeeper").
    def(init()).
    def(init<const TrueAverageBookkeeper<Inventory<Position<Security>>>&>()).
    def("record_transaction", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::RecordTransaction).
    def("get_inventory", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetInventory).
    def("get_total", &TrueAverageBookkeeper<
      Inventory<Position<Security>>>::GetTotal);
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
