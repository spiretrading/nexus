#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("Any") {
  TEST_CASE("equality") {
    REQUIRE(Any() == Any());
    REQUIRE(!(Any() != Any()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(Any(), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.size() == 1);
      auto matches = updates.front();
      updates.pop_front();
      REQUIRE(matches.m_additions.size() == 1);
      REQUIRE(matches.m_removals.empty());
      REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
    });
  }
}
