#include "Nexus/FeeHandlingTests/AsxtFeeHandlingTester.hpp"
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
using namespace std;

namespace {
  OrderFields BuildOrderFields(Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{"TST", DefaultMarkets::ASX(), DefaultCountries::AU()},
      DefaultCurrencies::AUD(), Side::BID, DefaultDestinations::ASXT(), 100,
      price);
    return fields;
  }

  AsxtFeeTable BuildFeeTable() {
    AsxtFeeTable feeTable;
    feeTable.m_gstRate = 1;
    feeTable.m_tradeRate = 1;
    feeTable.m_clearingRateTable[0] = rational<int>{1, 1000};
    feeTable.m_clearingRateTable[1] = rational<int>{1, 100};
    feeTable.m_clearingRateTable[2] = rational<int>{1, 10};
    return feeTable;
  }

  Money AsxtCalculateProcessingFee(const AsxtFeeTable& feeTable,
      const ExecutionReport& executionReport) {
    return CalculateFee(feeTable, executionReport).m_processingFee;
  }
}

void AsxtFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  TestFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::ACTIVE,
    AsxtCalculateProcessingFee, Money::ZERO);
}

void AsxtFeeHandlingTester::TestNoGst() {
  auto feeTable = BuildFeeTable();
  auto expectedFee = 100 * feeTable.m_clearingRateTable[2] * Money::ONE;
  TestFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    AsxtCalculateProcessingFee, expectedFee);
}
