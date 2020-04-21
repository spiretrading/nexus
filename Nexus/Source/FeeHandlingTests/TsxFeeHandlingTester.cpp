#include <doctest/doctest.h>
#include "Nexus/FeeHandling/TsxFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto BuildFeeTable() {
    auto feeTable = TsxFeeTable();
    PopulateFeeTable(Store(feeTable.m_continuousFeeTable));
    PopulateFeeTable(Store(feeTable.m_auctionFeeTable));
    PopulateFeeTable(Store(feeTable.m_oddLotFeeList));
    return feeTable;
  }

  auto BuildOrderFields(Money price) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::TSX(), DefaultCountries::CA()),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      price);
  }

  auto BuildHiddenOrderFields(Money price) {
    auto fields = BuildOrderFields(price);
    fields.m_type = OrderType::PEGGED;
    fields.m_additionalFields.emplace_back(18, "M");
    return fields;
  }
}

TEST_SUITE("TsxFeeHandling") {
  TEST_CASE("fee_table_calculations") {
    auto feeTable = BuildFeeTable();
    TestFeeTableIndex(feeTable, feeTable.m_continuousFeeTable,
      LookupContinuousFee, TsxFeeTable::PRICE_CLASS_COUNT,
      TsxFeeTable::TYPE_COUNT);
    TestFeeTableIndex(feeTable, feeTable.m_auctionFeeTable, LookupAuctionFee,
      TsxFeeTable::AUCTION_INDEX_COUNT, TsxFeeTable::AUCTION_TYPE_COUNT);
  }

  TEST_CASE("zero_quantity") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = Money::ZERO;
    auto fields = BuildOrderFields(Money::ONE);
    fields.m_quantity = 0;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::NONE,
      std::bind(CalculateFee, std::placeholders::_1,
      TsxFeeTable::Classification::DEFAULT, std::placeholders::_2,
      std::placeholders::_3), expectedFee);
  }

  TEST_CASE("active_interlisted_sub_dollar") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupContinuousFee(feeTable,
      TsxFeeTable::PriceClass::SUB_DOLLAR, TsxFeeTable::Type::ACTIVE);
    auto fields = BuildOrderFields(50 * Money::CENT);
    fields.m_quantity = 100;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::ACTIVE,
      std::bind(CalculateFee, std::placeholders::_1,
      TsxFeeTable::Classification::INTERLISTED, std::placeholders::_2,
      std::placeholders::_3), expectedFee);
  }

  TEST_CASE("passive_interlisted_sub_dollar") {
    auto feeTable = BuildFeeTable();
    auto expectedFee = LookupContinuousFee(feeTable,
      TsxFeeTable::PriceClass::SUB_DOLLAR, TsxFeeTable::Type::PASSIVE);
    auto fields = BuildOrderFields(50 * Money::CENT);
    fields.m_quantity = 100;
    TestPerShareFeeCalculation(feeTable, fields, LiquidityFlag::PASSIVE,
      std::bind(CalculateFee, std::placeholders::_1,
      TsxFeeTable::Classification::INTERLISTED, std::placeholders::_2,
      std::placeholders::_3), expectedFee);
  }
}
