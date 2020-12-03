#include <doctest/doctest.h>
#include "Nexus/Backtester/ActiveBacktesterEvent.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;

TEST_SUITE("BacktesterServiceClients") {
  TEST_CASE("test_timer") {
    auto localDataStore = LocalHistoricalDataStore();
    auto testEnvironment = TestEnvironment(MakeVirtualHistoricalDataStore(
      &localDataStore));
    auto startTime = time_from_string("2016-05-03 13:35:00");
    auto backtesterEnvironment = BacktesterEnvironment(startTime,
      ServiceClientsBox(std::in_place_type<TestServiceClients>,
        Ref(testEnvironment)));
    auto serviceClients = BacktesterServiceClients(Ref(backtesterEnvironment));
    auto timer = serviceClients.BuildTimer(seconds(21));
    REQUIRE(serviceClients.GetTimeClient().GetTime() == startTime);
    timer->Start();
    backtesterEnvironment.GetEventHandler().Add(
      std::make_shared<ActiveBacktesterEvent>(
      time_from_string("2016-05-07 00:00:00")));
    timer->Wait();
    REQUIRE(serviceClients.GetTimeClient().GetTime() ==
      startTime + seconds(21));
  }
}
