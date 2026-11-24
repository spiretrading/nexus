#include <doctest/doctest.h>
#include "Spire/Blotter/PortfolioSummaryModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Spire;

namespace {
  auto make_entry(const Ticker& ticker, CurrencyId currency, Quantity quantity,
      Money cost_basis, Money gross, Money fees, Money unrealized) {
    auto entry = PortfolioUpdateEntry();
    entry.m_inventory = Inventory(
      Position(ticker, currency, quantity, cost_basis), gross, fees, 0, 0);
    entry.m_unrealized = unrealized;
    entry.m_currency_inventory = Inventory(
      Position(ticker, currency, 1000, Money(10000)), Money(1000), Money(100),
      0, 0);
    entry.m_currency_unrealized = Money(1000);
    return entry;
  }

  void require_totals(const PortfolioUpdateEntry& value, CurrencyId currency,
      Money gross, Money fees, Money cost_basis, Money unrealized) {
    auto& inventory = value.m_currency_inventory;
    REQUIRE(inventory.m_position.m_currency == currency);
    REQUIRE(inventory.m_gross_profit_and_loss == gross);
    REQUIRE(inventory.m_fees == fees);
    REQUIRE(inventory.m_position.m_cost_basis == cost_basis);
    REQUIRE(value.m_currency_unrealized == unrealized);
  }
}

TEST_SUITE("PortfolioSummaryModel") {
  TEST_CASE("empty") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    auto model = PortfolioSummaryModel(portfolio, CAD, ExchangeRateTable());
    require_totals(
      model.get(), CAD, Money::ZERO, Money::ZERO, Money::ZERO, Money::ZERO);
    REQUIRE(model.set(model.get()) == QValidator::Invalid);
    REQUIRE(model.test(model.get()) == QValidator::Invalid);
  }

  TEST_CASE("snapshot") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    portfolio->push(make_entry(parse_ticker("RY.TSX"), CAD, 100, Money(1000),
      Money(20), Money(3), Money(5)));
    portfolio->push(make_entry(parse_ticker("TD.TSX"), CAD, -50, Money(-400),
      Money(10), Money(2), Money(-7)));
    auto model = PortfolioSummaryModel(portfolio, CAD, ExchangeRateTable());
    require_totals(
      model.get(), CAD, Money(30), Money(5), Money(1400), Money(-2));
  }

  TEST_CASE("updates") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    auto model = PortfolioSummaryModel(portfolio, CAD, ExchangeRateTable());
    auto updates = std::vector<PortfolioUpdateEntry>();
    auto connection =
      scoped_connection(model.connect_update_signal([&] (const auto& value) {
        updates.push_back(value);
      }));
    auto entry = make_entry(parse_ticker("RY.TSX"), CAD, 100, Money(1000),
      Money(20), Money(3), Money(5));
    portfolio->push(entry);
    require_totals(
      model.get(), CAD, Money(20), Money(3), Money(1000), Money(5));
    entry.m_unrealized = Money(15);
    portfolio->set(0, entry);
    require_totals(
      model.get(), CAD, Money(20), Money(3), Money(1000), Money(15));
    entry = make_entry(parse_ticker("RY.TSX"), CAD, 0, Money::ZERO, Money(50),
      Money(4), Money::ZERO);
    portfolio->set(0, entry);
    require_totals(
      model.get(), CAD, Money(50), Money(4), Money::ZERO, Money::ZERO);
    REQUIRE(updates.size() == 3);
    portfolio->remove(0);
    require_totals(model.get(), CAD, Money::ZERO, Money::ZERO, Money::ZERO,
      Money::ZERO);
    REQUIRE(updates.size() == 4);
  }

  TEST_CASE("transactions") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    auto entry = make_entry(parse_ticker("RY.TSX"), CAD, 100, Money(1000),
      Money(20), Money(3), Money(5));
    portfolio->push(entry);
    auto model = PortfolioSummaryModel(portfolio, CAD, ExchangeRateTable());
    auto previous = model.get();
    auto updates = std::vector<PortfolioUpdateEntry>();
    auto connection =
      scoped_connection(model.connect_update_signal([&] (const auto& value) {
        updates.push_back(value);
      }));
    portfolio->transact([&] {
      portfolio->remove(0);
      portfolio->transact([&] {
        entry.m_unrealized = Money(10);
        portfolio->push(entry);
      });
      REQUIRE(
        model.get().m_currency_inventory == previous.m_currency_inventory);
      REQUIRE(
        model.get().m_currency_unrealized == previous.m_currency_unrealized);
      REQUIRE(updates.empty());
    });
    REQUIRE(updates.size() == 1);
    require_totals(
      model.get(), CAD, Money(20), Money(3), Money(1000), Money(10));
    portfolio->transact([&] {
      portfolio->remove(0);
      portfolio->push(entry);
    });
    REQUIRE(updates.size() == 1);
    portfolio->set(0, entry);
    REQUIRE(updates.size() == 1);
    portfolio->push(make_entry(parse_ticker("TD.TSX"), CAD, 0, Money::ZERO,
      Money::ZERO, Money::ZERO, Money::ZERO));
    portfolio->move(0, 1);
    REQUIRE(updates.size() == 1);
    portfolio->remove(1);
    require_totals(
      model.get(), CAD, Money::ZERO, Money::ZERO, Money::ZERO, Money::ZERO);
    REQUIRE(updates.size() == 2);
  }

  TEST_CASE("currency_conversion") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    auto rates = ExchangeRateTable(
      {ExchangeRate(CurrencyPair(USD, CAD), rational<int>(5, 4))});
    portfolio->push(make_entry(parse_ticker("RY.TSX"), CAD, 20, Money(200),
      Money(10), Money(2), Money(4)));
    auto entry = make_entry(parse_ticker("IBM.NYSE"), USD, 100, Money(1000),
      Money(40), Money(4), Money(10));
    portfolio->push(entry);
    auto model = PortfolioSummaryModel(portfolio, CAD, rates);
    require_totals(model.get(), CAD, Money(60), Money(7), Money(1450),
      parse_money("16.50"));
    entry.m_inventory.m_position.m_currency = CAD;
    portfolio->set(1, entry);
    require_totals(
      model.get(), CAD, Money(50), Money(6), Money(1200), Money(14));
    portfolio->remove(1);
    require_totals(model.get(), CAD, Money(10), Money(2), Money(200), Money(4));
  }

  TEST_CASE("currency_rounding") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    auto rates = ExchangeRateTable(
      {ExchangeRate(CurrencyPair(USD, CAD), rational<int>(1, 2))});
    auto amount = parse_money("0.000001");
    for(auto& ticker : {parse_ticker("IBM.NYSE"), parse_ticker("F.NYSE")}) {
      portfolio->push(
        make_entry(ticker, USD, 1, amount, amount, amount, amount));
    }
    auto model = PortfolioSummaryModel(portfolio, CAD, rates);
    require_totals(model.get(), CAD, amount, amount, amount, amount);
  }

  TEST_CASE("missing_exchange_rate") {
    auto portfolio = std::make_shared<ArrayListModel<PortfolioUpdateEntry>>();
    portfolio->push(make_entry(parse_ticker("IBM.NYSE"), USD, 1, Money(100),
      Money::ZERO, Money::ZERO, Money::ZERO));
    REQUIRE_THROWS_AS(
      PortfolioSummaryModel(portfolio, CAD, ExchangeRateTable()),
      CurrencyPairNotFoundException);
  }
}
