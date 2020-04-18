#include <doctest/doctest.h>
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

namespace {
  auto GetTestSecurity() {
    return Security("TST", DefaultMarkets::HKEX(), DefaultCountries::HK());
  }

  auto BuildOrderFields(Money price) {
    return OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      GetTestSecurity(), DefaultCurrencies::HKD(), Side::BID,
      DefaultDestinations::HKEX(), 100, price);
  }
}

TEST_SUITE("HkexFeeHandling") {
  TEST_CASE("zero_quantity") {
    auto feeTable = HkexFeeTable();
  }

  TEST_CASE("default") {
    auto feeTable = HkexFeeTable();
  }
}
