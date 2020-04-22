#include <doctest/doctest.h>
#include "Nexus/FeeHandling/NexFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto GetTestSecurity() {
    return Security("TST", DefaultMarkets::TSXV(), DefaultCountries::CA());
  }

  auto BuildOrderFields(Money price) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::CAD(), Side::BID,
      DefaultDestinations::TSX(), 100, price);
  }

  auto BuildFeeTable() {
    auto feeTable = NexFeeTable();
    feeTable.m_fee = Money::ONE;
    return feeTable;
  }
}

TEST_SUITE("NexFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = BuildFeeTable();
    auto orderFields = BuildOrderFields(Money::ONE);
    TestPerShareFeeCalculation(feeTable, orderFields.m_price, 0,
      LiquidityFlag::NONE, std::bind(&CalculateFee, std::placeholders::_1,
      std::placeholders::_2), Money::ZERO);
  }

  TEST_CASE("execution") {
    auto feeTable = BuildFeeTable();
    auto orderFields = BuildOrderFields(Money::ONE);
    auto expectedFee = Money::ONE;
    TestPerShareFeeCalculation(feeTable, Money::ONE, 100, LiquidityFlag::ACTIVE,
      std::bind(&CalculateFee, std::placeholders::_1, std::placeholders::_2),
      expectedFee);
  }
}
