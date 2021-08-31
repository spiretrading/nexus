#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  using Foo = StateSelector<void, struct FooTag>;
  using Bar = StateSelector<void, struct BarTag>;
  using Baz = StateSelector<int, struct BazTag>;
  struct Update {
    std::unordered_set<const Stylist*> m_additions;
    std::unordered_set<const Stylist*> m_removals;
  };
}

TEST_SUITE("StateSelector") {
  TEST_CASE("equality") {
    REQUIRE(Foo() == Foo());
    REQUIRE(Baz(0) == Baz(0));
    REQUIRE(Baz(0) != Baz(1));
  }

  TEST_CASE("executor") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<Update>();
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

  TEST_CASE("executor_initialization") {
    run_test([] {
      auto w1 = QWidget();
      match(w1, Foo());
      auto updates = std::deque<Update>();
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

  TEST_CASE("selection") {
    run_test([] {
      auto w1 = QWidget();
      auto w2 = QWidget();
      auto w3 = QWidget();
      auto w4 = QWidget();
      match(w1, Foo());
      match(w2, Bar());
      match(w3, Baz(5));
      match(w4, Baz(10));
      auto stylists = std::unordered_set{&find_stylist(w1), &find_stylist(w2),
        &find_stylist(w3), &find_stylist(w4)};
      {
        auto selection = select(Foo(), stylists);
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w1)));
      }
      {
        auto selection = select(Bar(), stylists);
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
      {
        auto selection = select(Baz(5), stylists);
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w3)));
      }
      {
        auto selection = select(Baz(10), stylists);
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w4)));
      }
      {
        auto selection = select(Baz(20), stylists);
        REQUIRE(selection.empty());
      }
    });
  }
}
