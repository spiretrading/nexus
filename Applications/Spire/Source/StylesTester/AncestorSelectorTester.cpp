#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("AncestorSelector") {
  TEST_CASE("equality") {
    REQUIRE(AncestorSelector(Any(), Any()) == AncestorSelector(Any(), Any()));
    REQUIRE(AncestorSelector(Any(), Any()) != AncestorSelector(Any(), Hover()));
    REQUIRE(AncestorSelector(Any(), Any()) != AncestorSelector(Hover(), Any()));
    REQUIRE(AncestorSelector(Any(), Hover()) != AncestorSelector(Any(), Any()));
    REQUIRE(AncestorSelector(Hover(), Any()) != AncestorSelector(Any(), Any()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto root = QWidget();
      auto l1 = QWidget(&root);
      auto l2 = QWidget(&l1);
      auto l3 = QWidget(&l2);
      auto l4 = QWidget(&l3);
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(
        AncestorSelector(Hover(), Focus()), find_stylist(l3),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      match(l3, Hover());
      REQUIRE(updates.empty());
      match(l1, Focus());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(l1)));
      }
      unmatch(l3, Hover());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.empty());
        REQUIRE(matches.m_removals.size() == 1);
        REQUIRE(matches.m_removals.contains(&find_stylist(l1)));
      }
      match(l1, Focus());
      REQUIRE(updates.empty());
    });
  }
}
