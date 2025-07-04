#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AccountRoles.hpp"

using namespace Nexus;
using namespace Nexus::AdministrationService;

TEST_SUITE("AccountRoles") {
  TEST_CASE("stream") {
    auto ss = std::ostringstream();
    ss << AccountRole::TRADER;
    REQUIRE(ss.str() == "TRADER");
    ss.str("");
    ss << AccountRole::MANAGER;
    REQUIRE(ss.str() == "MANAGER");
    ss.str("");
    ss << AccountRole::SERVICE;
    REQUIRE(ss.str() == "SERVICE");
    ss.str("");
    ss << AccountRole::ADMINISTRATOR;
    REQUIRE(ss.str() == "ADMINISTRATOR");
    ss.str("");
    ss << AccountRole::NONE;
    REQUIRE(ss.str() == "NONE");
  }
}
