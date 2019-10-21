#include "Nexus/FeeHandlingTests/HkexFeeHandlingTester.hpp"
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/HkexFeeTable.hpp"
#include "Nexus/FeeHandlingTests/FeeTableTestUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tests;
using namespace std;

namespace {
  auto GetTestSecurity() {
    return Security("TST", DefaultMarkets::HKEX(), DefaultCountries::HK());
  }

  auto BuildOrderFields(Money price) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::HKD(), Side::BID,
      DefaultDestinations::HKEX(), 100, price);
    return fields;
  }
}

void HkexFeeHandlingTester::TestZeroQuantity() {
  auto feeTable = HkexFeeTable();
}

void HkexFeeHandlingTester::TestDefault() {
  auto feeTable = HkexFeeTable();
}
