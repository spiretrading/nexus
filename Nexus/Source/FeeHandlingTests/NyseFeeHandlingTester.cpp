#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/NyseFeeTable.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;

namespace {
  auto GetNyseSecurity() {
    return Security("TSTN", DefaultMarkets::NYSE(), DefaultCountries::US());
  }

  auto BuildFeeTable() {
    auto feeTable = NyseFeeTable();
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

TEST_SUITE("NyseFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = BuildFeeTable();
  }
}
