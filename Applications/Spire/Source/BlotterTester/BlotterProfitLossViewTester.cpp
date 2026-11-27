#include <boost/optional/optional_io.hpp>
#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/GroupedTableView.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/ToggleButton.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("BlotterProfitLossView") {
  UI_TEST_CASE("portfolio_columns") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    auto first = PortfolioUpdateEntry();
    first.m_inventory = Inventory(
      Position(parse_ticker("RY.TSX"), Currencies::CAD, 100, Money(1000)),
      Money(30), Money(2), 300, 3);
    first.m_unrealized = Money(-10);
    auto second = PortfolioUpdateEntry();
    second.m_inventory = Inventory(
      Position(parse_ticker("AAPL.NSDQ"), Currencies::USD, 0, Money::ZERO),
      Money(-20), Money(3), 200, 2);
    portfolio->push(first);
    portfolio->push(second);
    auto third = PortfolioUpdateEntry();
    third.m_inventory = Inventory(
      Position(parse_ticker("BMO.TSX"), Currencies::CAD, 100, Money(1000)),
      Money(10), Money(1), 100, 1);
    portfolio->push(third);
    auto view = BlotterProfitLossView(portfolio);
    REQUIRE(view.get_portfolio() == portfolio);
    auto& widget = static_cast<TableView&>(*view.focusProxy());
    auto& grouped = dynamic_cast<GroupedTableModel&>(*widget.get_table());
    REQUIRE(grouped.get_grouping() == std::vector<int>{0});
    auto& table = *grouped.get_source();
    REQUIRE(table.get_column_size() == 5);
    REQUIRE(table.get_row_size() == 3);
    REQUIRE(table.get<CurrencyId>(0, 0) == Currencies::CAD);
    REQUIRE(table.get<Ticker>(0, 1) == parse_ticker("RY.TSX"));
    REQUIRE(table.get<Money>(0, 2) == Money(18));
    REQUIRE(table.get<Money>(0, 3) == Money(2));
    REQUIRE(table.get<Quantity>(0, 4) == 300);
    REQUIRE(table.get<CurrencyId>(1, 0) == Currencies::USD);
    REQUIRE(table.get<Money>(1, 2) == Money(-23));
    REQUIRE(table.get<Quantity>(1, 4) == 200);
    auto& displayed = *widget.get_body().get_table();
    REQUIRE(displayed.get_row_size() == 5);
    REQUIRE(grouped.get_group_name(0) == "CAD");
    REQUIRE(displayed.get<Money>(0, 2) == Money(27));
    REQUIRE(displayed.get<Money>(0, 3) == Money(3));
    REQUIRE(displayed.get<Quantity>(0, 4) == 400);
    REQUIRE(displayed.get<Ticker>(1, 1) == parse_ticker("BMO.TSX"));
    REQUIRE(displayed.get<Ticker>(2, 1) == parse_ticker("RY.TSX"));
    REQUIRE(grouped.get_group_name(3) == "USD");
    REQUIRE(displayed.get<Money>(3, 2) == Money(-23));
    REQUIRE(displayed.get<Ticker>(4, 1) == parse_ticker("AAPL.NSDQ"));
    auto& selection = *widget.get_selection()->get_row_selection();
    REQUIRE(selection.push(1) == QValidator::Acceptable);
    REQUIRE(selection.push(4) == QValidator::Acceptable);
    REQUIRE(selection.get_size() == 2);
    widget.set_column_order({{2, SortedTableModel::Ordering::DESCENDING}});
    REQUIRE(displayed.get<Ticker>(1, 1) == parse_ticker("RY.TSX"));
    REQUIRE(displayed.get<Ticker>(2, 1) == parse_ticker("BMO.TSX"));
    widget.set_column_order({{4, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(displayed.get<Ticker>(1, 1) == parse_ticker("AAPL.NSDQ"));
    REQUIRE(displayed.get<Ticker>(3, 1) == parse_ticker("BMO.TSX"));
    REQUIRE(displayed.get<Ticker>(4, 1) == parse_ticker("RY.TSX"));
    REQUIRE(selection.get_size() == 2);
  }

  UI_TEST_CASE("live_profit_loss") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    auto view = BlotterProfitLossView(portfolio);
    auto& widget = static_cast<TableView&>(*view.focusProxy());
    auto& table = *widget.get_table();
    REQUIRE(table.get_row_size() == 0);
    auto entry = PortfolioUpdateEntry();
    entry.m_inventory = Inventory(
      Position(parse_ticker("RY.TSX"), Currencies::CAD, 100, Money(1000)),
      Money::ZERO, Money(2), 100, 1);
    entry.m_unrealized = Money(12);
    portfolio->push(entry);
    view.resize(600, 300);
    view.show();
    QApplication::processEvents();
    auto label = [&] (int row) {
      auto item = widget.get_body().find_item(TableIndex(row, 2));
      REQUIRE(item);
      for(auto child : item->get_body().findChildren<QWidget*>()) {
        if(auto money = dynamic_cast<MoneyBox*>(child)) {
          return money;
        }
      }
      return static_cast<MoneyBox*>(nullptr);
    };
    auto total = label(0);
    auto leaf = label(1);
    REQUIRE(total);
    REQUIRE(leaf);
    REQUIRE(total->is_read_only());
    REQUIRE(leaf->is_read_only());
    REQUIRE(total->get_current()->get() == Money(10));
    REQUIRE(leaf->get_current()->get() == Money(10));
    entry.m_unrealized = Money(-5);
    portfolio->set(0, entry);
    REQUIRE(table.get<Money>(0, 2) == Money(-7));
    REQUIRE(total->get_current()->get() == Money(-7));
    REQUIRE(leaf->get_current()->get() == Money(-7));
    entry.m_inventory.m_position.m_quantity = 0;
    entry.m_inventory.m_position.m_cost_basis = Money::ZERO;
    entry.m_inventory.m_gross_profit_and_loss = Money(20);
    entry.m_inventory.m_fees = Money(4);
    entry.m_inventory.m_volume = 200;
    entry.m_unrealized = Money::ZERO;
    portfolio->set(0, entry);
    REQUIRE(table.get_row_size() == 2);
    REQUIRE(table.get<Money>(0, 2) == Money(16));
    REQUIRE(table.get<Money>(0, 3) == Money(4));
    REQUIRE(table.get<Quantity>(0, 4) == 200);
    REQUIRE(total->get_current()->get() == Money(16));
    REQUIRE(leaf->get_current()->get() == Money(16));
    auto heading = widget.get_body().find_item(TableIndex(0, 0));
    REQUIRE(heading);
    auto toggle = dynamic_cast<ToggleButton*>(heading->get_body().focusProxy());
    REQUIRE(toggle);
    auto& displayed = *widget.get_body().get_table();
    toggle->get_current()->set(false);
    REQUIRE(displayed.get_row_size() == 1);
    auto other = entry;
    other.m_inventory.m_position.m_ticker = parse_ticker("BMO.TSX");
    portfolio->push(other);
    REQUIRE(displayed.get_row_size() == 1);
    REQUIRE(table.get_row_size() == 3);
    REQUIRE(total->get_current()->get() == Money(32));
    REQUIRE(table.get<Money>(0, 3) == Money(8));
    REQUIRE(table.get<Quantity>(0, 4) == 400);
    portfolio->remove(0);
    REQUIRE(displayed.get_row_size() == 1);
    REQUIRE(total->get_current()->get() == Money(16));
    toggle->get_current()->set(true);
    REQUIRE(displayed.get_row_size() == 2);
    REQUIRE(displayed.get<Ticker>(1, 1) == parse_ticker("BMO.TSX"));
    portfolio->remove(0);
    REQUIRE(table.get_row_size() == 0);
    REQUIRE(displayed.get_row_size() == 0);
  }
}
