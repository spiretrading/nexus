#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("OrSelector") {
  TEST_CASE("equality") {
    REQUIRE(OrSelector(Foo(), Foo()) == OrSelector(Foo(), Foo()));
    REQUIRE(OrSelector(Foo(), Foo()) != OrSelector(Foo(), Bar()));
    REQUIRE(OrSelector(Foo(), Foo()) != OrSelector(Bar(), Foo()));
    REQUIRE(OrSelector(Foo(), Bar()) != OrSelector(Foo(), Foo()));
    REQUIRE(OrSelector(Bar(), Foo()) != OrSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(OrSelector(Foo(), Bar()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      match(w1, Foo());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
      }
      match(w1, Bar());
      REQUIRE(updates.empty());
      unmatch(w1, Foo());
      unmatch(w1, Bar());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.empty());
        REQUIRE(matches.m_removals.size() == 1);
        REQUIRE(matches.m_removals.contains(&find_stylist(w1)));
      }
      match(w1, Bar());
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
      match(w1, Foo());
      match(w1, Bar());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(OrSelector(Foo(), Bar()), find_stylist(w1),
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
