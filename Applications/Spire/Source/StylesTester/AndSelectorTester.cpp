#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("AndSelector") {
  TEST_CASE("equality") {
    REQUIRE(AndSelector(Foo(), Foo()) == AndSelector(Foo(), Foo()));
    REQUIRE(AndSelector(Foo(), Foo()) != AndSelector(Foo(), Bar()));
    REQUIRE(AndSelector(Foo(), Foo()) != AndSelector(Bar(), Foo()));
    REQUIRE(AndSelector(Foo(), Bar()) != AndSelector(Foo(), Foo()));
    REQUIRE(AndSelector(Bar(), Foo()) != AndSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AndSelector(Foo(), Bar()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      match(w1, Foo());
      REQUIRE(updates.empty());
      unmatch(w1, Foo());
      REQUIRE(updates.empty());
      match(w1, Bar());
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
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.empty());
        REQUIRE(matches.m_removals.size() == 1);
        REQUIRE(matches.m_removals.contains(&find_stylist(w1)));
      }
      unmatch(w1, Bar());
      REQUIRE(updates.empty());
    });
  }

  TEST_CASE("select_initialization") {
    run_test([] {
      auto w1 = QWidget();
      match(w1, Foo());
      match(w1, Bar());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AndSelector(Foo(), Bar()), find_stylist(w1),
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
