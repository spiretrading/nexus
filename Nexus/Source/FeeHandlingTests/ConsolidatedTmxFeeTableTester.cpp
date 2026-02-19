#include <doctest/doctest.h>
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  auto make_ticker(std::string symbol, Venue venue) {
    return Ticker(std::move(symbol), venue);
  }

  auto make_order_fields(
      Ticker ticker, Money price, Quantity quantity = 100) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT, ticker,
      CAD, Side::BID, DefaultDestinations::TSX, quantity, price);
  }

  auto make_order(Ticker ticker, Money price, Quantity quantity = 100) {
    auto fields = make_order_fields(ticker, price, quantity);
    auto info = OrderInfo(
      fields, fields.m_account, 123, false, second_clock::universal_time());
    return std::make_shared<PrimitiveOrder>(info);
  }

  auto make_report(Money price, Quantity quantity,
      std::string liquidity_flag = "A", std::string last_market = "XTSE") {
    auto report = ExecutionReport(0, second_clock::universal_time());
    report.m_last_price = price;
    report.m_last_quantity = quantity;
    report.m_liquidity_flag = liquidity_flag;
    report.m_last_market = last_market;
    return report;
  }

  auto make_fee_table() {
    auto table = ConsolidatedTmxFeeTable();
    table.m_spire_fee = Money(1);
    table.m_iiroc_fee = Money(2);
    table.m_cds_fee = Money(3);
    table.m_cds_cap = 2;
    table.m_clearing_fee = Money(4);
    table.m_per_order_fee = Money(5);
    table.m_per_order_cap = Money(20);
    populate_fee_table(out(table.m_tsx_fee_table.m_continuous_fee_table));
    populate_fee_table(out(table.m_tsx_fee_table.m_auction_fee_table));
    populate_fee_table(out(table.m_tsx_fee_table.m_odd_lot_fee_list));
    populate_fee_table(out(table.m_tsxv_fee_table.m_continuous_fee_table));
    populate_fee_table(out(table.m_tsxv_fee_table.m_auction_fee_table));
    populate_fee_table(out(table.m_tsxv_fee_table.m_odd_lot_fee_list));
    populate_fee_table(out(table.m_xats_fee_table.m_general_fee_table));
    populate_fee_table(out(table.m_xats_fee_table.m_etf_fee_table));
    table.m_xats_fee_table.m_intraspread_dark_to_dark_max_fee = Money(10);
    table.m_xats_fee_table.m_intraspread_dark_to_dark_subdollar_max_fee =
      Money(20);
    populate_fee_table(out(table.m_chic_fee_table.m_fee_table));
    populate_fee_table(out(table.m_cse_fee_table.m_fee_table));
    populate_fee_table(out(table.m_cse2_fee_table.m_regular_table));
    populate_fee_table(out(table.m_cse2_fee_table.m_dark_table));
    populate_fee_table(out(table.m_cse2_fee_table.m_debentures_or_notes_table));
    populate_fee_table(
      out(table.m_cse2_fee_table.m_cse_listed_government_bonds_table));
    populate_fee_table(out(table.m_cse2_fee_table.m_oddlot_table));
    populate_fee_table(out(table.m_lynx_fee_table.m_fee_table));
    populate_fee_table(out(table.m_matn_fee_table.m_general_fee_table));
    populate_fee_table(out(table.m_matn_fee_table.m_alternative_fee_table));
    populate_fee_table(out(table.m_neoe_fee_table.m_general_fee_table));
    populate_fee_table(out(table.m_neoe_fee_table.m_interlisted_fee_table));
    populate_fee_table(out(table.m_neoe_fee_table.m_etf_table_fee));
    populate_fee_table(out(table.m_neoe_fee_table.m_neo_book_fee_table));
    table.m_nex_fee_table.m_fee = Money(7);
    populate_fee_table(out(table.m_omga_fee_table.m_fee_table));
    for(auto i = 0; i != PureFeeTable::SECTION_COUNT; ++i) {
      populate_fee_table(out(table.m_pure_fee_table.m_fee_table[i]));
    }
    populate_fee_table(out(table.m_xcx2_fee_table.m_default_table));
    populate_fee_table(out(table.m_xcx2_fee_table.m_tsx_table));
    table.m_etfs.insert(make_ticker("ETF", TSX));
    table.m_interlisted.insert(make_ticker("INT", TSX));
    table.m_nex_listed.insert(make_ticker("NEX", TSXV));
    return table;
  }
}

TEST_SUITE("consolidated_tmx_fee_table") {
  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto state = ConsolidatedTmxFeeTable::State();
    auto ticker = make_ticker("TST", TSX);
    auto order = make_order(ticker, Money::ONE, 0);
    auto report = make_report(Money::ONE, 0);
    auto result = calculate_fee(table, state, *order, report);
    REQUIRE(result.m_processing_fee == Money::ZERO);
    REQUIRE(result.m_commission == Money::ZERO);
    REQUIRE(result.m_execution_fee == Money::ZERO);
  }

  TEST_CASE("per_order_fee_cap") {
    auto table = make_fee_table();
    auto state = ConsolidatedTmxFeeTable::State();
    auto ticker = make_ticker("TST", TSX);
    auto order = make_order(ticker, Money::ONE, 100);
    auto report = make_report(Money::ONE, 10);
    auto result1 = calculate_fee(table, state, *order, report);
    auto result2 = calculate_fee(table, state, *order, report);
    auto per_order_total =
      state.m_per_order_charges.get(order->get_info().m_id);
    REQUIRE(per_order_total <= table.m_per_order_cap);
  }

  TEST_CASE("etf_fee_routing") {
    auto table = make_fee_table();
    auto state = ConsolidatedTmxFeeTable::State();
    auto ticker = make_ticker("ETF", TSX);
    auto order = make_order(ticker, Money::ONE, 100);
    auto report = make_report(Money::ONE, 100, "A", "XTSE");
    auto result = calculate_fee(table, state, *order, report);
    REQUIRE(result.m_execution_fee != Money::ZERO);
  }

  TEST_CASE("nex_listed_fee_routing") {
    auto table = make_fee_table();
    auto state = ConsolidatedTmxFeeTable::State();
    auto ticker = make_ticker("NEX", TSXV);
    auto order = make_order(ticker, Money::ONE, 100);
    auto report = make_report(Money::ONE, 100, "A", "XTSX");
    auto result = calculate_fee(table, state, *order, report);
    REQUIRE(result.m_execution_fee ==
      report.m_last_quantity * table.m_nex_fee_table.m_fee);
  }
}
