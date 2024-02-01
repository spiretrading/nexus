#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

namespace {
  using Baz = StateSelector<int, struct BazTag>;
}

TEST_SUITE("StateSelector") {
  TEST_CASE("equality") {
    REQUIRE(Foo() == Foo());
    REQUIRE(Baz(0) == Baz(0));
    REQUIRE(Baz(0) != Baz(1));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(Foo(), find_stylist(w1),
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
      match(w1, Foo());
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
    });
  }

  TEST_CASE("select_initialization") {
    run_test([] {
      auto w1 = QWidget();
      match(w1, Foo());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(Foo(), find_stylist(w1),
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
