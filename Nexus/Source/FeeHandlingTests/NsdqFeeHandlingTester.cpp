#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/NsdqFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto GetUsSecurity() {
    return Security("TST", DefaultMarkets::NASDAQ(), DefaultCountries::US());
  }

  auto BuildFeeTable() {
    auto feeTable = NsdqFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

TEST_SUITE("NsdqFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = BuildFeeTable();
    TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
      CalculateFee, Money::ZERO);
  }
}
