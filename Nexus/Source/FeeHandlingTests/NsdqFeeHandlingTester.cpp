#include "Nexus/FeeHandlingTests/NsdqFeeHandlingTester.hpp"
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
using namespace std;

namespace {
  Security GetUsSecurity() {
    return {"TST", DefaultMarkets::NASDAQ(), DefaultCountries::US()};
  }

  NsdqFeeTable BuildFeeTable() {
    NsdqFeeTable feeTable;
    PopulateFeeTable(Store(feeTable.m_feeTable));
    feeTable.m_subDollarRate = {30, 10000};
    return feeTable;
  }
}

void NsdqFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  TestPerShareFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::NONE,
    CalculateFee, Money::ZERO);
}
