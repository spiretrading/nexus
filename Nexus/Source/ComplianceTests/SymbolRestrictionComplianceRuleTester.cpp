#include <doctest/doctest.h>
#include "Nexus/Compliance/SymbolRestrictionComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;

namespace {
  auto MakeOrderFields(std::string symbol, MarketCode market) {
    auto& marketEntry = GetDefaultMarketDatabase().FromCode(market);
    return OrderFields::MakeLimitOrder(DirectoryEntry::GetRootAccount(),
      Security(std::move(symbol), market, marketEntry.m_countryCode),
      DefaultCurrencies::CAD(), Side::BID, DefaultDestinations::TSX(), 100,
      Money::ONE);
  }
}

TEST_SUITE("SymbolRestrictionComplianceRule") {
  TEST_CASE("empty_restriction") {
    auto rule = SymbolRestrictionComplianceRule(
      std::vector<ComplianceParameter>());
    auto order = PrimitiveOrder({MakeOrderFields("TST1",
      DefaultMarkets::TSX()), 1, second_clock::universal_time()});
    rule.Submit(order);
  }

  TEST_CASE("single_symbol_submission_and_restriction") {
    auto symbols = std::vector<ComplianceValue>();
    symbols.push_back(Security("TST1", DefaultMarkets::TSX(),
      DefaultCountries::CA()));
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("symbols", symbols);
    auto rule = SymbolRestrictionComplianceRule(parameters);
    {
      auto order = PrimitiveOrder({MakeOrderFields("TST1",
        DefaultMarkets::TSX()), 1, second_clock::universal_time()});
      REQUIRE_THROWS_AS(rule.Submit(order), ComplianceCheckException);
    }
    {
      auto order = PrimitiveOrder({MakeOrderFields("TST2",
        DefaultMarkets::TSX()), 2, second_clock::universal_time()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
    {
      auto order = PrimitiveOrder({MakeOrderFields("TST1",
        DefaultMarkets::ASX()), 3, second_clock::universal_time()});
      REQUIRE_NOTHROW(rule.Submit(order));
    }
  }
}
