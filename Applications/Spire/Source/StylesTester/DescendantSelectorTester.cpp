#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("DescendantSelector") {
  TEST_CASE("equality") {
    REQUIRE(
      DescendantSelector(Any(), Any()) == DescendantSelector(Any(), Any()));
    REQUIRE(
      DescendantSelector(Any(), Any()) != DescendantSelector(Any(), Hover()));
    REQUIRE(
      DescendantSelector(Any(), Any()) != DescendantSelector(Hover(), Any()));
    REQUIRE(
      DescendantSelector(Any(), Hover()) != DescendantSelector(Any(), Any()));
    REQUIRE(
      DescendantSelector(Hover(), Any()) != DescendantSelector(Any(), Any()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto selector = DescendantSelector(Any(), Any());
      auto root = QWidget();
      auto l1 = QWidget(&root);
      auto l2 = QWidget(&l1);
      auto l3 = QWidget(&l2);
      auto l4 = QWidget(&l3);
      {
        auto selection = select(selector, {&find_stylist(l4)});
        REQUIRE(selection.empty());
      }
      {
        auto selection = select(selector, {&find_stylist(l3)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(l4)));
      }
      {
        auto selection = select(selector, {&find_stylist(l2)});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(l3)));
        REQUIRE(selection.contains(&find_stylist(l4)));
      }
      {
        auto selection = select(selector, {&find_stylist(l1)});
        REQUIRE(selection.size() == 3);
        REQUIRE(selection.contains(&find_stylist(l2)));
        REQUIRE(selection.contains(&find_stylist(l3)));
        REQUIRE(selection.contains(&find_stylist(l4)));
      }
      {
        auto selection = select(selector, {&find_stylist(root)});
        REQUIRE(selection.size() == 4);
        REQUIRE(selection.contains(&find_stylist(l1)));
        REQUIRE(selection.contains(&find_stylist(l2)));
        REQUIRE(selection.contains(&find_stylist(l3)));
        REQUIRE(selection.contains(&find_stylist(l4)));
      }
    });
  }
}
