#include <boost/optional/optional_io.hpp>
#include "Spire/Blotter/BlotterProfitLossView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("BlotterProfitLossView") {
  UI_TEST_CASE("portfolio_columns") {
    initialize_resources();
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
    auto view = BlotterProfitLossView(portfolio);
    REQUIRE(view.get_portfolio() == portfolio);
    auto& widget = static_cast<TableView&>(*view.focusProxy());
    auto& table = *widget.get_table();
    REQUIRE(table.get_column_size() == 5);
    REQUIRE(table.get_row_size() == 2);
    REQUIRE(table.get<CurrencyId>(0, 0) == Currencies::CAD);
    REQUIRE(table.get<Ticker>(0, 1) == parse_ticker("RY.TSX"));
    REQUIRE(table.get<Money>(0, 2) == Money(18));
    REQUIRE(table.get<Money>(0, 3) == Money(2));
    REQUIRE(table.get<Quantity>(0, 4) == 300);
    REQUIRE(table.get<CurrencyId>(1, 0) == Currencies::USD);
    REQUIRE(table.get<Money>(1, 2) == Money(-23));
    REQUIRE(table.get<Quantity>(1, 4) == 200);
    auto& displayed =
      static_cast<SortedTableModel&>(*widget.get_body().get_table());
    REQUIRE(displayed.get<Ticker>(0, 1) == parse_ticker("AAPL.NSDQ"));
    displayed.set_column_order({{2, SortedTableModel::Ordering::DESCENDING}});
    REQUIRE(displayed.get<Ticker>(0, 1) == parse_ticker("RY.TSX"));
    displayed.set_column_order({{4, SortedTableModel::Ordering::ASCENDING}});
    REQUIRE(displayed.get<Ticker>(0, 1) == parse_ticker("AAPL.NSDQ"));
  }

  UI_TEST_CASE("live_profit_loss") {
    initialize_resources();
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
    auto label = static_cast<MoneyBox*>(nullptr);
    for(auto child : view.findChildren<QWidget*>()) {
      if(auto money = dynamic_cast<MoneyBox*>(child)) {
        label = money;
        break;
      }
    }
    REQUIRE(label);
    REQUIRE(label->is_read_only());
    REQUIRE(label->get_current()->get() == Money(10));
    entry.m_unrealized = Money(-5);
    portfolio->set(0, entry);
    REQUIRE(table.get<Money>(0, 2) == Money(-7));
    REQUIRE(label->get_current()->get() == Money(-7));
    entry.m_inventory.m_position.m_quantity = 0;
    entry.m_inventory.m_position.m_cost_basis = Money::ZERO;
    entry.m_inventory.m_gross_profit_and_loss = Money(20);
    entry.m_inventory.m_fees = Money(4);
    entry.m_inventory.m_volume = 200;
    entry.m_unrealized = Money::ZERO;
    portfolio->set(0, entry);
    REQUIRE(table.get_row_size() == 1);
    REQUIRE(table.get<Money>(0, 2) == Money(16));
    REQUIRE(table.get<Money>(0, 3) == Money(4));
    REQUIRE(table.get<Quantity>(0, 4) == 200);
    REQUIRE(label->get_current()->get() == Money(16));
    portfolio->remove(0);
    REQUIRE(table.get_row_size() == 0);
  }
}
