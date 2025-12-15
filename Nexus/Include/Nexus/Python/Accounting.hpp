#ifndef NEXUS_PYTHON_ACCOUNTING_HPP
#define NEXUS_PYTHON_ACCOUNTING_HPP
#include <string_view>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include <pybind11/pybind11.h>
#include "Nexus/Accounting/Bookkeeper.hpp"
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported Bookkeeper. */
  NEXUS_EXPORT_DLL pybind11::class_<Bookkeeper>& get_exported_bookkeeper();

  /** Returns the exported Portfolio. */
  NEXUS_EXPORT_DLL pybind11::class_<Portfolio<Bookkeeper>>&
    get_exported_portfolio();

  /**
   * Exports the Accounting namespace.
   * @param module The module to export to.
   */
  void export_accounting(pybind11::module& module);

  /**
   * Exports the Bookkeeper class.
   * @param <B> The type of bookkeeper to export.
   * @param module The module to export to.
   */
  template<IsBookkeeper B>
  auto export_bookkeeper(pybind11::module& module, std::string_view name) {
    auto bookkeeper = pybind11::class_<B>(module, name.data()).
      def("record", &B::record, pybind11::arg("index"),
        pybind11::arg("currency"), pybind11::arg("quantity"),
        pybind11::arg("cost_basis"), pybind11::arg("fees")).
      def("get_inventory", &B::get_inventory, pybind11::arg("index"),
        pybind11::arg("currency")).
      def("get_total", &B::get_total, pybind11::arg("currency")).
      def_property_readonly("inventories", &B::get_inventory_range).
      def_property_readonly("totals", &B::get_totals_range);
    if constexpr(!std::is_same_v<B, Bookkeeper>) {
      pybind11::implicitly_convertible<B, Bookkeeper>();
      get_exported_bookkeeper().
        def(pybind11::init<B*>(), pybind11::keep_alive<1, 2>());
    }
    return bookkeeper;
  }

  /**
   * Exports the bookkeeper reactor.
   * @param module The module to export to.
   */
  void export_bookkeeper_reactor(pybind11::module& module);

  /**
   * Exports the BuyingPowerModel class.
   * @param module The module to export to.
   */
  void export_buying_power_model(pybind11::module& module);

  /**
   * Exports the Inventory struct.
   * @param module The module to export to.
   */
  void export_inventory(pybind11::module& module);

  /**
   * Exports the Portfolio class.
   * @param <B> The type of Bookkeeper.
   * @param module The module to export to.
   * @param name The name used to export the class.
   * @return The exported class.
   */
  template<IsBookkeeper B>
  auto export_portfolio(pybind11::module& module, std::string_view name) {
    using Portfolio = Portfolio<B>;
    using Bookkeeper = typename Portfolio::Bookkeeper;
    auto portfolio = pybind11::class_<Portfolio>(module, name.data()).
      def(pybind11::init<const Bookkeeper&>()).
      def(pybind11::init<const Bookkeeper&, const VenueDatabase&>()).
      def_property_readonly("bookkeeper", &Portfolio::get_bookkeeper).
      def_property_readonly("security_entries",
        &Portfolio::get_security_entries).
      def_property_readonly("unrealized_profit_and_losses",
        &Portfolio::get_unrealized_profit_and_losses).
      def("update", pybind11::overload_cast<
        const OrderFields&, const ExecutionReport&>(&Portfolio::update),
        pybind11::arg("fields"), pybind11::arg("report")).
      def("update_ask", &Portfolio::update_ask, pybind11::arg("security"),
        pybind11::arg("value")).
      def("update_bid", &Portfolio::update_bid, pybind11::arg("security"),
        pybind11::arg("value")).
      def("update", pybind11::overload_cast<const Security&, Money, Money>(
        &Portfolio::update), pybind11::arg("security"),
        pybind11::arg("ask_value"), pybind11::arg("bid_value")).
      def("__iter__", [] (const Portfolio& portfolio) {
        auto updates = std::vector<PortfolioUpdateEntry>();
        for_each(portfolio, [&] (const PortfolioUpdateEntry& update) {
          updates.push_back(update);
        });
        return pybind11::make_iterator(updates.begin(), updates.end());
      }, pybind11::keep_alive<0, 1>());
    if constexpr(std::is_same_v<Bookkeeper, Nexus::Bookkeeper>) {
      portfolio.def(pybind11::init([] {
          return Portfolio(Bookkeeper(TrueAverageBookkeeper()));
        })).
        def(pybind11::init([] (const VenueDatabase& venues) {
          return Portfolio(Bookkeeper(TrueAverageBookkeeper()), venues);
        }));
    } else {
      portfolio.def(pybind11::init()).
        def(pybind11::init<const VenueDatabase&>());
    }
    using SecurityEntry = typename Portfolio::SecurityEntry;
    pybind11::class_<SecurityEntry>(portfolio, "SecurityEntry").
      def(pybind11::init<CurrencyId>()).
      def_readwrite("valuation", &SecurityEntry::m_valuation).
      def_readwrite("unrealized", &SecurityEntry::m_unrealized);
    module.def("get_realized_profit_and_loss",
      pybind11::overload_cast<const Inventory&>(&get_realized_profit_and_loss));
    module.def("get_unrealized_profit_and_loss",
      [] (const Inventory& inventory, const SecurityValuation& valuation) {
        return get_unrealized_profit_and_loss(inventory, valuation);
      });
    module.def("get_total_profit_and_loss",
      [] (const Inventory& inventory, const SecurityValuation& valuation) {
        return get_total_profit_and_loss(inventory, valuation);
      });
    module.def("get_total_profit_and_loss",
      [] (const Portfolio& portfolio, CurrencyId currency) {
        return get_total_profit_and_loss(portfolio, currency);
      });
    return portfolio;
  }

  /**
   * Exports the PortfolioController class.
   * @param module The module to export to.
   */
  void export_portfolio_controller(pybind11::module& module);

  /**
   * Exports the PortfolioUpdateEntry struct.
   * @param module The module to export to.
   */
  void export_portfolio_update_entry(pybind11::module& module);

  /**
   * Exports the Position struct.
   * @param module The module to export to.
   */
  void export_position(pybind11::module& module);

  /**
   * Exports the PositionOrderBook class.
   * @param module The module to export to.
   */
  void export_position_order_book(pybind11::module& module);

  /**
   * Exports the SecurityValuation struct.
   * @param module The module to export to.
   */
  void export_security_valuation(pybind11::module& module);

  /**
   * Exports the ShortingModel class.
   * @param module The module to export to.
   */
  void export_shorting_model(pybind11::module& module);
}

#endif
