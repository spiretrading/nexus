#include <Beam/ServicesTests/TestServices.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskController.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::RiskService;

namespace {
  auto ACCOUNT_A = DirectoryEntry::MakeAccount(153, "simba");
  auto ACCOUNT_B = DirectoryEntry::MakeAccount(155, "mufassa");
  auto TSLA = Security("TSLA", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  auto XIU = Security("XIU", DefaultMarkets::TSX(), DefaultCountries::CA());

  struct Fixture {
    TestEnvironment m_environment;

    Fixture() {}
  };
}

TEST_SUITE("RiskController") {
  TEST_CASE_FIXTURE(Fixture, "single_account") {
  }
}
