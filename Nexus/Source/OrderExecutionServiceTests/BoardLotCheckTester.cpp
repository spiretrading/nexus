#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <doctest/doctest.h>
#ifdef FAIL
  #undef FAIL
#endif
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/OrderExecutionService/BoardLotCheck.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;

namespace {
  struct Fixture {
    using BoardLotCheck = BoardLotCheck<VirtualMarketDataClient*>;

    TestEnvironment m_environment;
    TestServiceClients m_serviceClients;
    BoardLotCheck m_check;

    Fixture()
        : m_serviceClients(Ref(m_environment)),
          m_check(&m_serviceClients.GetMarketDataClient(),
            GetDefaultMarketDatabase(), GetDefaultTimeZoneDatabase()) {
      m_environment.Open();
      m_serviceClients.Open();
    }
  };
}

TEST_SUITE("BoardLotCheck") {
  TEST_CASE_FIXTURE(Fixture, "unavailable_bbo_quote") {
    auto security = Security("TST", DefaultMarkets::TSX(),
      DefaultCountries::CA());
    auto orderInfoA = OrderInfo(OrderFields::BuildLimitOrder(
      DirectoryEntry::GetRootAccount(), security, DefaultCurrencies::CAD(),
      Side::BID, "TSX", 100, Money::ONE), 1,
      m_environment.GetTimeEnvironment().GetTime());
    auto orderA = PrimitiveOrder(orderInfoA);
    REQUIRE_THROWS_AS(m_check.Submit(orderInfoA),
      OrderSubmissionCheckException);
    m_environment.Publish(security,
      BboQuote(Quote(Money::ONE, 100, Side::BID),
      Quote(Money::ONE + Money::CENT, 100, Side::ASK), not_a_date_time));
    REQUIRE_NOTHROW(m_check.Submit(orderInfoA));
  }
}
