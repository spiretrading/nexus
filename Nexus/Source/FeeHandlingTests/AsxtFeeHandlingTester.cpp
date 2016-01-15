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

  Money AsxtCalculateFee(const AsxtFeeTable& feeTable,
      const OrderFields& orderFields, const ExecutionReport& executionReport) {
    return CalculateFee(feeTable, executionReport);
  }
}

void AsxtFeeHandlingTester::TestZeroQuantity() {
  AsxtFeeTable feeTable;
  feeTable.m_gstRate = 1;
  feeTable.m_tradeRate = 1;
  TestFeeCalculation(feeTable, Money::ONE, 0, LiquidityFlag::ACTIVE,
    CalculateFee, Money::ZERO);
}

void AsxtFeeHandlingTester::TestNoGst() {
  AsxtFeeTable feeTable;
  feeTable.m_gstRate = 0;
  feeTable.m_tradeRate = 1;
  auto expectedFee = 100 * Money::ONE;
  TestFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    CalculateFee, expectedFee);
}
