#include <doctest/doctest.h>
#include "Nexus/FeeHandling/TsxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  const auto AUCTION_MAX_QUANTITY = Quantity(10000);

  auto make_fee_table() {
    auto table = TsxFeeTable();
    populate_fee_table(out(table.m_continuous_fee_table));
    populate_fee_table(out(table.m_auction_fee_table));
    table.m_auction_fee_table[static_cast<int>(
      TsxFeeTable::AuctionType::OPEN)][static_cast<int>(
        TsxFeeTable::AuctionIndex::MAX_CHARGE)] = AUCTION_MAX_QUANTITY *
          lookup_auction_fee(table, TsxFeeTable::AuctionIndex::FEE,
            TsxFeeTable::AuctionType::OPEN);
    table.m_auction_fee_table[static_cast<int>(
      TsxFeeTable::AuctionType::CLOSE)][static_cast<int>(
        TsxFeeTable::AuctionIndex::MAX_CHARGE)] = AUCTION_MAX_QUANTITY *
          lookup_auction_fee(table, TsxFeeTable::AuctionIndex::FEE,
            TsxFeeTable::AuctionType::CLOSE);
    populate_fee_table(out(table.m_odd_lot_fee_list));
    return table;
  }

  auto make_order_fields(Money price) {
    return make_limit_order_fields(DirectoryEntry::ROOT_ACCOUNT,
      Security("TST", DefaultVenues::TSX), DefaultCurrencies::CAD, Side::BID,
      DefaultDestinations::TSX, 100, price);
  }

  auto make_hidden_order_fields(Money price) {
    auto fields = make_order_fields(price);
    fields.m_type = OrderType::PEGGED;
    fields.m_additional_fields.emplace_back(18, "M");
    return fields;
  }
}

TEST_SUITE("TsxFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto table = make_fee_table();
    test_fee_table_index(table, table.m_continuous_fee_table,
      lookup_continuous_fee, TsxFeeTable::PRICE_CLASS_COUNT,
      TsxFeeTable::TYPE_COUNT);
    test_fee_table_index(table, table.m_auction_fee_table, lookup_auction_fee,
      TsxFeeTable::AUCTION_INDEX_COUNT, TsxFeeTable::AUCTION_TYPE_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto table = make_fee_table();
    auto expected_fee = Money::ZERO;
    auto fields = make_order_fields(Money::ONE);
    fields.m_quantity = 0;
    test_per_share_fee_calculation(table, fields, LiquidityFlag::NONE,
      expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(
          table, TsxFeeTable::Classification::DEFAULT, fields, report);
      });
  }

  TEST_CASE("active_interlisted_subdollar") {
    auto table = make_fee_table();
    auto expected_fee = lookup_continuous_fee(table,
      TsxFeeTable::PriceClass::SUBDOLLAR, TsxFeeTable::Type::ACTIVE);
    auto fields = make_order_fields(50 * Money::CENT);
    fields.m_quantity = 100;
    test_per_share_fee_calculation(table, fields, LiquidityFlag::ACTIVE,
      expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(
          table, TsxFeeTable::Classification::INTERLISTED, fields, report);
      });
  }

  TEST_CASE("passive_interlisted_subdollar") {
    auto table = make_fee_table();
    auto expected_fee = lookup_continuous_fee(table,
      TsxFeeTable::PriceClass::SUBDOLLAR, TsxFeeTable::Type::PASSIVE);
    auto fields = make_order_fields(50 * Money::CENT);
    fields.m_quantity = 100;
    test_per_share_fee_calculation(table, fields, LiquidityFlag::PASSIVE,
      expected_fee,
      [] (const auto& table, const auto& fields, const auto& report) {
        return calculate_fee(table, TsxFeeTable::Classification::INTERLISTED,
          fields, report);
      });
  }

  TEST_CASE("opening_auction") {
    auto table = make_fee_table();
    auto per_share_fee = lookup_auction_fee(table,
      TsxFeeTable::AuctionIndex::FEE, TsxFeeTable::AuctionType::OPEN);
    auto max_fee = lookup_auction_fee(table,
      TsxFeeTable::AuctionIndex::MAX_CHARGE, TsxFeeTable::AuctionType::OPEN);
    {
      auto quantity = Quantity(100);
      auto fields = make_order_fields(5 * Money::ONE);
      fields.m_quantity = quantity;
      test_per_share_fee_calculation(table, fields, "O", per_share_fee,
        [] (const auto& table, const auto& fields, const auto& report) {
          return calculate_fee(
            table, TsxFeeTable::Classification::DEFAULT, fields, report);
        });
    }
    {
      auto quantity = AUCTION_MAX_QUANTITY + 100;
      auto fields = make_order_fields(Money::CENT);
      fields.m_quantity = quantity;
      test_fee_calculation(table, fields, "O", max_fee,
        [] (const auto& table, const auto& fields, const auto& report) {
          return calculate_fee(table, TsxFeeTable::Classification::DEFAULT,
            fields, report);
        });
    }
  }
}
