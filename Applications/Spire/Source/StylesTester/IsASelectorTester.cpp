#include <doctest/doctest.h>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/IsASelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

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

  TEST_CASE("derived_selection") {
    run_test([] {
      auto selector = IsASelector(std::in_place_type<B>);
      auto a = A();
      auto b = B();
      auto selection = select(selector, {&find_stylist(a), &find_stylist(b)});
      REQUIRE(selection.size() == 1);
      REQUIRE(selection.contains(&find_stylist(b)));
    });
  }

  TEST_CASE("parent_selection") {
    run_test([] {
      auto selector = IsASelector(std::in_place_type<A>);
      auto a = A();
      auto b = B();
      auto selection = select(selector, {&find_stylist(a), &find_stylist(b)});
      REQUIRE(selection.size() == 2);
      REQUIRE(selection.contains(&find_stylist(a)));
      REQUIRE(selection.contains(&find_stylist(b)));
    });
  }

  TEST_CASE("base_selection") {
    run_test([] {
      auto selector = IsASelector(std::in_place_type<QWidget>);
      auto a = A();
      auto b = B();
      auto selection = select(selector, {&find_stylist(a), &find_stylist(b)});
      REQUIRE(selection.size() == 2);
      REQUIRE(selection.contains(&find_stylist(a)));
      REQUIRE(selection.contains(&find_stylist(b)));
    });
  }
}
