#include "Spire/CanvasTests/TranslationTester.hpp"
#include <Beam/Threading/TimerThreadPool.hpp>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/Canvas/ControlNodes/ChainNode.hpp"
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

namespace {
  struct Environment {
    TimerThreadPool m_timerThreadPool;
    TestEnvironment m_environment;
    std::unique_ptr<VirtualServiceClients> m_serviceClients;
    UserProfile m_userProfile;

    Environment()
      : m_serviceClients(MakeVirtualServiceClients(
          std::make_unique<TestServiceClients>(Ref(m_environment)))),
        m_userProfile("", false, false, GetDefaultCountryDatabase(),
          GetDefaultTimeZoneDatabase(), GetDefaultCurrencyDatabase(), {},
          GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(),
          EntitlementDatabase(), Ref(m_timerThreadPool),
          Ref(*m_serviceClients)) {
      m_environment.Open();
      m_serviceClients->Open();
    }
  };
}

void TranslationTester::TestTranslatingConstant() {
  auto environment = Environment();
  auto value = IntegerNode(100);
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
    Ref(executor), DirectoryEntry());
  auto translation = Translate(context, value);
  auto result = translation.Extract<Aspen::Box<Quantity>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::COMPLETE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 100);
}

void TranslationTester::TestTranslatingChain() {
  auto environment = Environment();
  auto chain = std::unique_ptr<CanvasNode>(std::make_unique<ChainNode>());
  chain = chain->Replace("i0", std::make_unique<IntegerNode>(123));
  chain = chain->Replace("i1", std::make_unique<IntegerNode>(456));
  auto executor = Executor();
  auto context = CanvasNodeTranslationContext(Ref(environment.m_userProfile),
    Ref(executor), DirectoryEntry());
  auto translation = Translate(context, *chain);
  auto result = translation.Extract<Aspen::Box<Quantity>>();
  CPPUNIT_ASSERT(result.commit(0) == Aspen::State::CONTINUE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 123);
  CPPUNIT_ASSERT(result.commit(1) == Aspen::State::CONTINUE_EVALUATED);
  CPPUNIT_ASSERT(result.eval() == 456);
}
