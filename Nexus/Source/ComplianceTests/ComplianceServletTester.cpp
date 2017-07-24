#include "Nexus/ComplianceTests/ComplianceServletTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace std;

namespace {
  OrderFields BuildOrderFields(string symbol, MarketCode market) {
    auto fields = OrderFields::BuildLimitOrder(DirectoryEntry::GetRootAccount(),
      Security{std::move(symbol), market, DefaultCountries::CA()},
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
    return fields;
  }
}

void ComplianceServletTester::setUp() {}

void ComplianceServletTester::tearDown() {}
