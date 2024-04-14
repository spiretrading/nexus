#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Spire;

TEST_CASE("ListModelIterator") {
  auto list = ArrayListModel<int>();
  list.push(1);
  list.push(2);
  list.push(3);
  list.push(4);
  list.push(5);
  auto i = list.begin();
  SUBCASE("dereferece") {
    REQUIRE_UNARY(*i == 1);
  }
  SUBCASE("increment") {
    ++i;
    REQUIRE_UNARY(*i == 2);
  }
  SUBCASE("random_access") {
    auto j = i + 3;
    REQUIRE_UNARY(*j == 4);
  }
}
