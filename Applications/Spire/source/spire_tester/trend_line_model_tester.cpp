#include <catch.hpp>
#include "spire/spire_tester/spire_tester.hpp"
#include "spire/charting/trend_line.hpp"
#include "spire/charting/trend_line_model.hpp"

using namespace Spire;

TEST_CASE("", "[]") {
  auto model = TrendLineModel();
  REQUIRE(true == true);
  print_test_name("test_name");
}
