#include <deque>
#include <doctest/doctest.h>
#include "Spire/Styles/IsASelector.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

namespace {
  struct A : QWidget {
    using QWidget::QWidget;
  };

  struct B : A {
    using A::A;
  };
}

TEST_SUITE("IsASelector") {
  TEST_CASE("equality") {
    REQUIRE(
      IsASelector(std::in_place_type<A>) == IsASelector(std::in_place_type<A>));
    REQUIRE(
      IsASelector(std::in_place_type<A>) != IsASelector(std::in_place_type<B>));
  }

  TEST_CASE("select_exact") {
    run_test([] {
      auto b = B();
      auto updates = std::deque<SelectionUpdate>();
      auto connection =
        select(IsASelector(std::in_place_type<B>), find_stylist(b),
          [&] (const auto& additions, const auto& removals) {
            updates.push_back({additions, removals});
          });
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(b)));
      }
    });
  }

  TEST_CASE("select_base_from_derived") {
    run_test([] {
      auto a = A();
      auto updates = std::deque<SelectionUpdate>();
      auto connection =
        select(IsASelector(std::in_place_type<B>), find_stylist(a),
          [&] (const auto& additions, const auto& removals) {
            updates.push_back({additions, removals});
          });
      REQUIRE(updates.empty());
    });
  }

  TEST_CASE("select_derived_from_parent") {
    run_test([] {
      auto b = B();
      auto updates = std::deque<SelectionUpdate>();
      auto connection =
        select(IsASelector(std::in_place_type<A>), find_stylist(b),
          [&] (const auto& additions, const auto& removals) {
            updates.push_back({additions, removals});
          });
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(b)));
      }
    });
  }

  TEST_CASE("select_derived_from_base") {
    run_test([] {
      auto b = B();
      auto updates = std::deque<SelectionUpdate>();
      auto connection =
        select(IsASelector(std::in_place_type<QWidget>), find_stylist(b),
          [&] (const auto& additions, const auto& removals) {
            updates.push_back({additions, removals});
          });
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(b)));
      }
    });
  }
}
