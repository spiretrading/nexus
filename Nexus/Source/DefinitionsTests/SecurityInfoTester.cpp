#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"

using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("SecurityInfo") {
  TEST_CASE("stream") {
    auto info =
      SecurityInfo(Security("ABC", NYSE), "Test Company", "Technology", 100);
    auto ss = std::ostringstream();
    ss << info;
    auto expected = std::ostringstream();
    expected << '(' << info.m_security << ' ' << info.m_name << ' '<<
      info.m_sector << ' ' << info.m_board_lot << ')';
    REQUIRE(ss.str() == expected.str());
  }
}
