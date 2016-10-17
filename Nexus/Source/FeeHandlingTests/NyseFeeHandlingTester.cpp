#include "Nexus/FeeHandlingTests/NyseFeeHandlingTester.hpp"
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
using namespace std;

namespace {
  Security GetNyseSecurity() {
    return {"TSTN", DefaultMarkets::NYSE(), DefaultCountries::US()};
  }

  NyseFeeTable BuildFeeTable() {
    NyseFeeTable feeTable;
    feeTable.m_subDollarRate = {30, 10000};
    PopulateFeeTable(Store(feeTable.m_feeTable));
    return feeTable;
  }
}

void NyseFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = BuildFeeTable();
}
