#include <doctest/doctest.h>
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
    auto testServiceClients = MakeVirtualServiceClients(
      std::make_unique<TestServiceClients>(Ref(testEnvironment)));
    auto startTime = ptime(date(2020, 05, 03), time_duration(13, 35, 0));
    auto backtesterEnvironment = BacktesterEnvironment(startTime,
      Ref(*testServiceClients));
    auto serviceClients = BacktesterServiceClients(Ref(backtesterEnvironment));
    auto timer = serviceClients.BuildTimer(seconds(21));
    REQUIRE(serviceClients.GetTimeClient().GetTime() == startTime);
    timer->Start();
    timer->Wait();
    REQUIRE(serviceClients.GetTimeClient().GetTime() ==
      startTime + seconds(21));
  }
}
