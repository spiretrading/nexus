#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;

namespace {
  using TestPortfolio = Portfolio<TrueAverageBookkeeper>;
  auto TST = parse_ticker("TST.TSX");

  auto make_order_fields(
      Ticker ticker, Asset currency, Side side, Quantity quantity) {
    auto fields = OrderFields();
    fields.m_ticker = ticker;
    fields.m_currency = currency;
    fields.m_side = side;
    fields.m_quantity = quantity;
    return fields;
  }

  auto make_execution_report(Quantity last_quantity, Money last_price) {
    auto report = ExecutionReport();
    report.m_last_quantity = last_quantity;
    report.m_last_price = last_price;
    return report;
  }
}

TEST_SUITE("Portfolio") {
  TEST_CASE("empty_portfolio") {
    auto portfolio = TestPortfolio();
    REQUIRE(portfolio.get_entries().empty());
    REQUIRE(portfolio.get_unrealized_profit_and_losses().empty());
  }

  TEST_CASE("constructor") {
    auto bookkeeper = TrueAverageBookkeeper();
    bookkeeper.record(TST, CAD, 1, Money::ONE, Money::ZERO);
    auto portfolio = Portfolio(bookkeeper);
    REQUIRE(portfolio.get_entries().at(TST).m_unrealized == Money::ZERO);
    portfolio.update(TST, 3 * Money::ONE, 2 * Money::ONE);
    REQUIRE(portfolio.get_unrealized_profit_and_losses().at(CAD) == Money::ONE);
    REQUIRE(portfolio.get_entries().at(TST).m_unrealized == Money::ONE);
  }

  TEST_CASE("update_with_fill_and_valuation") {
    auto portfolio = TestPortfolio();
    auto fields = make_order_fields(TST, CAD, Side::BID, 100);
    auto report = make_execution_report(100, 10 * Money::ONE);
    portfolio.update(fields, report);
    auto& bookkeeper = portfolio.get_bookkeeper();
    auto& inventory = bookkeeper.get_inventory(TST);
    REQUIRE(inventory.m_position.m_quantity == 100);
    REQUIRE(inventory.m_position.m_cost_basis == 1000 * Money::ONE);
    REQUIRE(portfolio.get_unrealized_profit_and_losses().empty());
    portfolio.update_bid(TST, 10.50 * Money::ONE);
    auto& entries = portfolio.get_entries();
    REQUIRE(entries.size() == 1);
    auto& entry = entries.at(TST);
    REQUIRE(entry.m_unrealized == 50 * Money::ONE);
    auto& unrealized_pnls = portfolio.get_unrealized_profit_and_losses();
    REQUIRE(unrealized_pnls.size() == 1);
    REQUIRE(unrealized_pnls.at(CAD) == 50 * Money::ONE);
  }

  TEST_CASE("short_position_pnl") {
    auto portfolio = TestPortfolio();
    auto fields = make_order_fields(TST, CAD, Side::ASK, 100);
    auto report = make_execution_report(100, 10 * Money::ONE);
    portfolio.update(fields, report);
    portfolio.update_ask(TST, 9.50 * Money::ONE);
    auto& entries = portfolio.get_entries();
    REQUIRE(entries.at(TST).m_unrealized == 50 * Money::ONE);
    auto& unrealized_pnls = portfolio.get_unrealized_profit_and_losses();
    REQUIRE(unrealized_pnls.at(CAD) == 50 * Money::ONE);
  }

  TEST_CASE("get_realized_profit_and_loss") {
    auto inventory = Inventory();
    inventory.m_gross_profit_and_loss = 100 * Money::ONE;
    inventory.m_fees = 10 * Money::ONE;
    REQUIRE(get_realized_profit_and_loss(inventory) == 90 * Money::ONE);
  }

  TEST_CASE("get_unrealized_profit_and_loss") {
    auto inventory = Inventory();
    inventory.m_position.m_quantity = 100;
    inventory.m_position.m_cost_basis = 1000 * Money::ONE;
    auto valuation = Valuation(CAD);
    valuation.m_bid_value = 12 * Money::ONE;
    auto pnl = get_unrealized_profit_and_loss(inventory, valuation);
    REQUIRE(pnl);
    REQUIRE(*pnl == 200 * Money::ONE);
  }

  TEST_CASE("get_total_profit_and_loss_inventory") {
    auto inventory = Inventory();
    inventory.m_position.m_quantity = 100;
    inventory.m_position.m_cost_basis = 1000 * Money::ONE;
    inventory.m_gross_profit_and_loss = 50 * Money::ONE;
    inventory.m_fees = 5 * Money::ONE;
    auto valuation = Valuation(CAD);
    valuation.m_bid_value = 12 * Money::ONE;
    auto pnl = get_total_profit_and_loss(inventory, valuation);
    REQUIRE(pnl);
    REQUIRE(*pnl == (45 + 200) * Money::ONE);
  }

  TEST_CASE("get_total_profit_and_loss_portfolio") {
    auto portfolio = TestPortfolio();
    auto fields = make_order_fields(TST, CAD, Side::BID, 100);
    auto report = make_execution_report(100, 10 * Money::ONE);
    portfolio.update(fields, report);
    portfolio.update_bid(TST, 12 * Money::ONE);
    auto pnl = get_total_profit_and_loss(portfolio, CAD);
    REQUIRE(pnl == 200 * Money::ONE);
  }

  TEST_CASE("for_each") {
    auto portfolio = TestPortfolio();
    auto fields = make_order_fields(TST, CAD, Side::BID, 100);
    auto report = make_execution_report(100, 10 * Money::ONE);
    portfolio.update(fields, report);
    portfolio.update_bid(TST, 11 * Money::ONE);
    auto update_count = 0;
    for_each(portfolio, [&] (const auto& update) {
      ++update_count;
      REQUIRE(update.m_inventory.m_position.m_quantity == 100);
      REQUIRE(update.m_unrealized == 100 * Money::ONE);
      REQUIRE(update.m_currency_inventory.m_position.m_quantity == 100);
      REQUIRE(update.m_unrealized_currency == 100 * Money::ONE);
    });
    REQUIRE(update_count == 1);
  }
}
