#include "Nexus/FeeHandlingTests/NexFeeHandlingTester.hpp"
#include "Nexus/FeeHandling/NexFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  Security GetTestSecurity() {
    return Security{"TST", DefaultMarkets::TSXV(), DefaultCountries::CA()};
  }

  OrderFields BuildOrderFields(Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::TSX(), 100, price);
    return fields;
  }

  NexFeeTable BuildFeeTable() {
    NexFeeTable feeTable;
    feeTable.m_fee = Money::ONE;
    return feeTable;
  }
}

void NexFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
  auto orderFields = BuildOrderFields(Money::ONE);
  TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
    LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1,
    std::placeholders::_2), Money::ZERO);
}

void NexFeeHandlingTester::TestExecution() {
  auto feeTable = BuildFeeTable();
  auto orderFields = BuildOrderFields(Money::ONE);
  auto expectedFee = Money::ONE;
  TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
    std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
    expectedFee);
}
