#ifndef NEXUS_PYTHON_ACCOUNTING_HPP
#define NEXUS_PYTHON_ACCOUNTING_HPP
#include <string_view>
#include <Beam/Python/Utilities.hpp>
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Nexus/Accounting/Bookkeeper.hpp"
#include "Nexus/Accounting/Inventory.hpp"
#include "Nexus/Accounting/Position.hpp"

namespace pybind11 {
  inline std::ostream& pybind11::operator <<(
      std::ostream& out, const object& value) {
    return out << str(value).cast<std::string>();
  }
}

namespace Nexus::Python {

  /**
   * Exports the Accounting namespace.
   * @param module The module to export to.
   */
  void export_accounting(pybind11::module& module);

  template<Accounting::IsBookkeeper Bookkeeper>
  auto export_bookkeeper(pybind11::module& module, std::string_view name) {
    using Index = typename Bookkeeper::Index;
    auto bookkeeper = pybind11::class_<Bookkeeper>(module, name.data()).
      def("record", &Bookkeeper::record, pybind11::arg("index"),
        pybind11::arg("currency"), pybind11::arg("quantity"),
        pybind11::arg("cost_basis"), pybind11::arg("fees")).
      def("get_inventory", &Bookkeeper::get_inventory, pybind11::arg("index"),
        pybind11::arg("currency")).
      def("get_total", &Bookkeeper::get_total, pybind11::arg("currency")).
      def_property_readonly("inventories", &Bookkeeper::get_inventory_range).
      def_property_readonly("totals", &Bookkeeper::get_totals_range);
    if constexpr(!std::is_same_v<Index, pybind11::object>) {
      pybind11::implicitly_convertible<Accounting::Bookkeeper<
        Accounting::Inventory<Accounting::Position<pybind11::object>>>,
          Bookkeeper>();
    }
    if constexpr(!std::is_same_v<Bookkeeper, Accounting::Bookkeeper<
        Accounting::Inventory<Accounting::Position<Index>>>>) {
      pybind11::implicitly_convertible<Bookkeeper, Accounting::Bookkeeper<
        Accounting::Inventory<Accounting::Position<Index>>>>();
      if constexpr(!std::is_same_v<Index, pybind11::object>) {
        pybind11::implicitly_convertible<Bookkeeper, Accounting::Bookkeeper<
          Accounting::Inventory<Accounting::Position<pybind11::object>>>>();
      }
    }
    return bookkeeper;
  }

  /**
   * Exports the Inventory class.
   * @param <P> The type of Position.
   * @param module The module to export to.
   * @param name The name used to export the class.
   * @return The exported class.
   */
  template<Accounting::IsPosition P>
  auto export_inventory(pybind11::module& module, std::string_view name) {
    using Inventory = Accounting::Inventory<P>;
    auto inventory = pybind11::class_<Inventory>(module, name.data()).
      def(pybind11::init()).
      def(pybind11::init<typename Inventory::Position::Key>()).
      def(pybind11::init<typename Inventory::Position, Money, Money, Quantity,
        int>()).
      def_readwrite("position", &Inventory::m_position).
      def_readwrite("gross_profit_and_loss",
        &Inventory::m_gross_profit_and_loss).
      def_readwrite("fees", &Inventory::m_fees).
      def_readwrite("volume", &Inventory::m_volume).
      def_readwrite("transaction_count", &Inventory::m_transaction_count).
      def(pybind11::self == pybind11::self).
      def(pybind11::self != pybind11::self).
      def("__str__", &boost::lexical_cast<std::string, Inventory>);
    module.def("is_empty", &Accounting::is_empty<P>);
    if constexpr(!std::is_same_v<typename P::Index, pybind11::object>) {
      pybind11::implicitly_convertible<
        Accounting::Position<pybind11::object>, Inventory>();
    }
    return inventory;
  }

  /**
   * Exports the Position class.
   * @param <I> The type of Index.
   * @param module The module to export to.
   * @param name The name used to export the class.
   * @return The exported class.
   */
  template<typename I>
  auto export_position(pybind11::module& module, std::string_view name) {
    using Position = Accounting::Position<I>;
    using Key = typename Position::Key;
    auto position = pybind11::class_<Position>(module, name.data()).
      def(pybind11::init()).
      def(pybind11::init<const Position&>()).
      def(pybind11::init<const Key&, Quantity, Money>()).
      def_readwrite("key", &Position::m_key).
      def_readwrite("quantity", &Position::m_quantity).
      def_readwrite("cost_basis", &Position::m_cost_basis).
      def("__str__", &boost::lexical_cast<std::string, Position>).
      def(pybind11::self == pybind11::self).
      def(pybind11::self != pybind11::self);
    pybind11::class_<Key>(position, "Key").
      def(pybind11::init()).
      def(pybind11::init<const Key&>()).
      def(pybind11::init<const typename Key::Index&, CurrencyId>()).
      def_readwrite("index", &Key::m_index).
      def_readwrite("currency", &Key::m_currency).
      def("__str__", &boost::lexical_cast<std::string, Key>).
      def(pybind11::self == pybind11::self).
      def(pybind11::self != pybind11::self).
      def("__hash__", [] (const Key& key) {
        if constexpr(std::is_same_v<I, pybind11::object>) {
          return pybind11::hash(key.m_index);
        } else {
          return std::hash<Key>()(key);
        }
      });
    module.def("average_price", &Accounting::get_average_price<I>);
    module.def("side", &Accounting::get_side<I>);
    if constexpr(!std::is_same_v<I, pybind11::object>) {
      pybind11::implicitly_convertible<
        Accounting::Position<pybind11::object>, Position>();
    }
  }

#if 0
  /**
   * Exports the BuyingPowerModel class.
   * @param module The module to export to.
   */
  void ExportBuyingPowerModel(pybind11::module& module);

  /**
   * Exports the PortfolioUpdateEntry class.
   * @param module The module to export to.
   */
  void ExportPortfolioUpdateEntry(pybind11::module& module);

  /**
   * Exports the PositionOrderBook class.
   * @param module The module to export to.
   */
  void ExportPositionOrderBook(pybind11::module& module);

  /**
   * Exports the Inventory<Position<Security>> class.
   * @param module The module to export to.
   */
  void ExportSecurityInventory(pybind11::module& module);

  /**
   * Exports the SecurityValuation class.
   * @param module The module to export to.
   */
  void ExportSecurityValuation(pybind11::module& module);

  /**
   * Exports the TrueAverageBookkeeper class.
   * @param module The module to export to.
   */
  void ExportTrueAverageBookkeeper(pybind11::module& module);

  /**
   * Exports the BookkeeperReactor using a TrueAverageBookkeeper.
   * @param module The module to export to.
   */
  void ExportTrueAverageBookkeeperReactor(pybind11::module& module);

  /**
   * Exports the TrueAveragePortfolio class.
   * @param module The module to export to.
   */
  void ExportTrueAveragePortfolio(pybind11::module& module);
#endif
}

#endif
