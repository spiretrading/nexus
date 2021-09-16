#include <deque>
#include <doctest/doctest.h>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("NotSelector") {
  TEST_CASE("equality") {
    REQUIRE(NotSelector(Foo()) == NotSelector(Foo()));
    REQUIRE(NotSelector(Foo()) != NotSelector(Bar()));
    REQUIRE(NotSelector(Bar()) != NotSelector(Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      match(w1, Foo());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(NotSelector(Foo()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      unmatch(w1, Foo());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
      }
    });
  }

  TEST_CASE("select_initialization") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(NotSelector(Foo()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
      }
      match(w1, Foo());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.empty());
        REQUIRE(matches.m_removals.size() == 1);
        REQUIRE(matches.m_removals.contains(&find_stylist(w1)));
      }
    });
  }
}
