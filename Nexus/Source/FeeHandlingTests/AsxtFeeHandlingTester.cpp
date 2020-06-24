#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/AsxtFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto BuildOrderFields(Money price) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security("TST", DefaultMarkets::ASX(), DefaultCountries::AU()),
      DefaultCurrencies::AUD(), Side::BID, DefaultDestinations::ASXT(), 100,
      price);
  }

  auto BuildPeggedOrderFields(Money price) {
    auto fields = BuildOrderFields(price);
    fields.m_type = OrderType::PEGGED;
    return fields;
  }

  auto BuildFeeTable() {
    auto feeTable = AsxtFeeTable();
    feeTable.m_gstRate = 1;
    feeTable.m_tradeRate = 1;
    feeTable.m_clearingRateTable[0][0] = rational<int>{1, 1000};
    feeTable.m_clearingRateTable[0][1] = rational<int>{1, 100};
    feeTable.m_clearingRateTable[0][2] = rational<int>{1, 10};
    feeTable.m_clearingRateTable[1][0] = rational<int>{5, 1000};
    feeTable.m_clearingRateTable[1][1] = rational<int>{5, 100};
    feeTable.m_clearingRateTable[1][2] = rational<int>{5, 10};
    return feeTable;
  }
}

TEST_SUITE("AsxtFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = BuildFeeTable();
    auto fields = BuildOrderFields(Money::ONE);
    fields.m_quantity = 0;
    TestNotionalValueFeeCalculation(feeTable, fields,
      std::bind(CalculateClearingFee, std::placeholders::_1, fields,
      std::placeholders::_2), 0);
    TestNotionalValueFeeCalculation(feeTable, fields, CalculateExecutionFee, 0);
  }

  TEST_CASE("regular_tier_one") {
    auto feeTable = BuildFeeTable();
    auto fields = BuildOrderFields(5 * Money::CENT);
    fields.m_quantity = 100;
    TestNotionalValueFeeCalculation(feeTable, fields,
      std::bind(CalculateClearingFee, std::placeholders::_1, fields,
      std::placeholders::_2), LookupClearingFee(feeTable,
      AsxtFeeTable::PriceClass::TIER_ONE,
      AsxtFeeTable::OrderTypeClass::REGULAR));
    TestFeeCalculation(feeTable, fields, "A", CalculateExecutionFee,
      (1 + feeTable.m_gstRate) * feeTable.m_tradeRate * fields.m_quantity *
      fields.m_price);
  }

  TEST_CASE("regular_tier_two") {
    auto feeTable = BuildFeeTable();
    auto fields = BuildOrderFields(50 * Money::CENT);
    fields.m_quantity = 500;
    TestNotionalValueFeeCalculation(feeTable, fields,
      std::bind(CalculateClearingFee, std::placeholders::_1, fields,
      std::placeholders::_2), LookupClearingFee(feeTable,
      AsxtFeeTable::PriceClass::TIER_TWO,
      AsxtFeeTable::OrderTypeClass::REGULAR));
    TestFeeCalculation(feeTable, fields, "R", CalculateExecutionFee,
      (1 + feeTable.m_gstRate) * feeTable.m_tradeRate * fields.m_quantity *
      fields.m_price);
  }

  TEST_CASE("regular_tier_three") {
    auto feeTable = BuildFeeTable();
    auto fields = BuildOrderFields(Money::ONE);
    fields.m_quantity = 12000;
    TestNotionalValueFeeCalculation(feeTable, fields,
      std::bind(CalculateClearingFee, std::placeholders::_1, fields,
      std::placeholders::_2), LookupClearingFee(feeTable,
      AsxtFeeTable::PriceClass::TIER_THREE,
      AsxtFeeTable::OrderTypeClass::REGULAR));
    TestFeeCalculation(feeTable, fields, "R", CalculateExecutionFee,
      (1 + feeTable.m_gstRate) * feeTable.m_tradeRate * fields.m_quantity *
      fields.m_price);
  }

  TEST_CASE("pegged_tier_one") {
    auto feeTable = BuildFeeTable();
    auto fields = BuildPeggedOrderFields(5 * Money::CENT);
    fields.m_quantity = 6000;
    TestNotionalValueFeeCalculation(feeTable, fields,
      std::bind(CalculateClearingFee, std::placeholders::_1, fields,
      std::placeholders::_2), LookupClearingFee(feeTable,
      AsxtFeeTable::PriceClass::TIER_ONE,
      AsxtFeeTable::OrderTypeClass::PEGGED));
    TestFeeCalculation(feeTable, fields, "R", CalculateExecutionFee,
      (1 + feeTable.m_gstRate) * feeTable.m_tradeRate * fields.m_quantity *
      fields.m_price);
  }

  TEST_CASE("pegged_tier_two") {
    auto feeTable = BuildFeeTable();
    auto fields = BuildPeggedOrderFields(50 * Money::CENT);
    fields.m_quantity = 300;
    TestNotionalValueFeeCalculation(feeTable, fields,
      std::bind(CalculateClearingFee, std::placeholders::_1, fields,
      std::placeholders::_2), LookupClearingFee(feeTable,
      AsxtFeeTable::PriceClass::TIER_TWO,
      AsxtFeeTable::OrderTypeClass::PEGGED));
    TestFeeCalculation(feeTable, fields, "A", CalculateExecutionFee,
      (1 + feeTable.m_gstRate) * feeTable.m_tradeRate * fields.m_quantity *
      fields.m_price);
  }

  TEST_CASE("pegged_tier_three") {
    auto feeTable = BuildFeeTable();
    auto fields = BuildPeggedOrderFields(Money::ONE);
    fields.m_quantity = 4400;
    TestNotionalValueFeeCalculation(feeTable, fields,
      std::bind(CalculateClearingFee, std::placeholders::_1, fields,
      std::placeholders::_2), LookupClearingFee(feeTable,
      AsxtFeeTable::PriceClass::TIER_THREE,
      AsxtFeeTable::OrderTypeClass::PEGGED));
    TestFeeCalculation(feeTable, fields, "R", CalculateExecutionFee,
      (1 + feeTable.m_gstRate) * feeTable.m_tradeRate * fields.m_quantity *
      fields.m_price);
  }
}
