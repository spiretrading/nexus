#include <Beam/Utilities/ToString.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/Ticker.hpp"

using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Venues;

namespace {
  using TestPortfolio = Portfolio<TrueAverageBookkeeper>;

  auto TST = parse_ticker("TST.TSX");

  auto make_order_fields(
      Ticker ticker, CurrencyId currency, Side side, Quantity quantity) {
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
  TEST_CASE("format_valuation") {
    auto valuation = Valuation(CAD);
    auto test = [&] (const std::string& expected) {
      REQUIRE(std::format("{}", valuation) == expected);
      REQUIRE(Beam::to_string(valuation) == expected);
    };
    test("(CAD None None)");
    valuation.m_ask_value = Money::ZERO;
    test("(CAD 0.00 None)");
    valuation.m_bid_value = parse_money("12.25");
    test("(CAD 0.00 12.25)");
    valuation.m_ask_value = boost::none;
    test("(CAD None 12.25)");
    REQUIRE(std::format("{:*>18}", valuation) == "**(CAD None 12.25)");
    REQUIRE(std::format("{:.4}", valuation) == "(CAD");
  }

  TEST_CASE("format_update") {
    auto inventory = Inventory(Position(TST, CAD, 100, Money(1000)),
      Money(50), Money::CENT, 200, 2);
    auto total = Inventory(Position(Ticker(), CAD, 0, Money::ZERO),
      Money(50), Money::CENT, 200, 2);
    auto update = PortfolioUpdateEntry(inventory, Money(25), total, Money(30));
    auto expected = std::string(
      "(((TST.TSX CAD 100 1000.00) 50.00 0.01 200 2) 25.00 "
      "(( CAD 0 0.00) 50.00 0.01 200 2) 30.00)");
    REQUIRE(std::format("{}", update) == expected);
    REQUIRE(Beam::to_string(update) == expected);
    REQUIRE(std::format("{:*>{}}", update, expected.size() + 2) ==
      "**" + expected);
    REQUIRE(std::format("{:.6}", update) == "(((TST");
  }

  TEST_CASE("format_databases") {
    auto currencies = CurrencyDatabase();
    auto currency = CurrencyDatabase::Entry();
    currency.m_id = CAD;
    currency.m_code = "CUS";
    currencies.add(currency);
    auto venues = VenueDatabase();
    auto venue = VenueDatabase::Entry();
    venue.m_venue = TSX;
    venue.m_display_name = "VENUE";
    venues.add(venue);
    auto locale = make_locale(currencies, venues);
    auto test = [&] (const auto& value, const std::string& expected) {
      REQUIRE(std::format(locale, "{}", value) == expected);
      auto output = std::ostringstream();
      output.imbue(locale);
      output << value;
      REQUIRE(output.str() == expected);
      output.str("");
      output << CURRENCIES << VENUES << value;
      REQUIRE(output.str() == Beam::to_string(value));
      output.str("");
      output.imbue(std::locale::classic());
      output << currencies << venues << value;
      REQUIRE(output.str() == expected);
    };
    auto position = Position(TST, CAD, -100, Money(-1000));
    test(position, "(TST.VENUE CUS -100 -1000.00)");
    auto inventory = Inventory(position, Money(50), Money::CENT, 200, 2);
    test(inventory, "((TST.VENUE CUS -100 -1000.00) 50.00 0.01 200 2)");
    test(PositionOrderBook::PositionEntry(TST, -100), "(TST.VENUE -100)");
    test(Valuation(CAD), "(CUS None None)");
    auto update = PortfolioUpdateEntry(
      inventory, Money(25), inventory, Money(30));
    test(update, "(((TST.VENUE CUS -100 -1000.00) 50.00 0.01 200 2) 25.00 "
      "((TST.VENUE CUS -100 -1000.00) 50.00 0.01 200 2) 30.00)");
  }

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
      REQUIRE(update.m_currency_unrealized == 100 * Money::ONE);
    });
    REQUIRE(update_count == 1);
  }
}
