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
    REQUIRE(AndSelector(Any(), Any()) == AndSelector(Any(), Any()));
    REQUIRE(AndSelector(Any(), Any()) != AndSelector(Any(), Hover()));
    REQUIRE(AndSelector(Any(), Any()) != AndSelector(Hover(), Any()));
    REQUIRE(AndSelector(Any(), Hover()) != AndSelector(Any(), Any()));
    REQUIRE(AndSelector(Hover(), Any()) != AndSelector(Any(), Any()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AndSelector(Hover(), Focus()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      match(w1, Hover());
      REQUIRE(updates.empty());
      unmatch(w1, Hover());
      REQUIRE(updates.empty());
      match(w1, Focus());
      REQUIRE(updates.empty());
      match(w1, Hover());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
      }
      match(w1, Focus());
      REQUIRE(updates.empty());
      unmatch(w1, Hover());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.empty());
        REQUIRE(matches.m_removals.size() == 1);
        REQUIRE(matches.m_removals.contains(&find_stylist(w1)));
      }
      unmatch(w1, Focus());
      REQUIRE(updates.empty());
    });
  }

  TEST_CASE("select_initialization") {
    run_test([] {
      auto w1 = QWidget();
      match(w1, Hover());
      match(w1, Focus());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AndSelector(Hover(), Focus()), find_stylist(w1),
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