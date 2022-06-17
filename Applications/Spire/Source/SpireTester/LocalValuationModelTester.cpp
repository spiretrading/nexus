#include <doctest/doctest.h>
#include "Spire/Blotter/LocalValuationModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Spire;

TEST_SUITE("LocalValuationModel") {
  TEST_CASE("construction") {
    auto tst_valuation = std::make_shared<CompositeModel<SecurityValuation>>();
    tst_valuation->add(&SecurityValuation::m_currency,
      make_constant_value_model(DefaultCurrencies::USD()));
    auto tst_bid = std::make_shared<LocalValueModel<optional<Money>>>();
    tst_valuation->add(&SecurityValuation::m_bidValue, tst_bid);
    auto tst_ask = std::make_shared<LocalValueModel<optional<Money>>>();
    tst_valuation->add(&SecurityValuation::m_askValue, tst_ask);
    auto model = LocalValuationModel();
    auto tst = ParseSecurity("TST.NYSE");
    model.add(tst, tst_valuation);
    REQUIRE(model.get_valuation(tst) == tst_valuation);
    auto foo = model.get_valuation(ParseSecurity("FOO.NYSE"));
    REQUIRE(foo != nullptr);
    REQUIRE(foo != tst_valuation);
  }
}
