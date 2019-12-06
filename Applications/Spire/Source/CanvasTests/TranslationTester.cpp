#include "Spire/CanvasTests/TranslationTester.hpp"
#include <Beam/Threading/TimerThreadPool.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Tasks/Executor.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;
using namespace Spire::Tests;

void TranslationTester::TestTranslatingConstant() {
  auto timerThreadPool = TimerThreadPool();
  auto environment = TestEnvironment();
  environment.Open();
  auto serviceClients = MakeVirtualServiceClients(
    std::make_unique<TestServiceClients>(Ref(environment)));
  serviceClients->Open();
  auto userProfile = UserProfile("", false, false, GetDefaultCountryDatabase(),
    GetDefaultTimeZoneDatabase(), GetDefaultCurrencyDatabase(), {},
    GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(),
    EntitlementDatabase(), Ref(timerThreadPool), Ref(*serviceClients));
  auto value = IntegerNode(100);
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(userProfile), Ref(executor),
    DirectoryEntry());
  auto translation = Translate(context, value);
  auto result = translation.Extract<Aspen::Box<Quantity>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 100);
}
