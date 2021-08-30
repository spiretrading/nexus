#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("AncestorSelector") {
  TEST_CASE("equality") {
    REQUIRE(AncestorSelector(Any(), Any()) == AncestorSelector(Any(), Any()));
    REQUIRE(AncestorSelector(Any(), Any()) != AncestorSelector(Any(), Hover()));
    REQUIRE(AncestorSelector(Any(), Any()) != AncestorSelector(Hover(), Any()));
    REQUIRE(AncestorSelector(Any(), Hover()) != AncestorSelector(Any(), Any()));
    REQUIRE(AncestorSelector(Hover(), Any()) != AncestorSelector(Any(), Any()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto selector = AncestorSelector(Any(), Any());
      auto root = new QWidget();
      auto l1 = new QWidget(root);
      auto l2 = new QWidget(l1);
      auto l3 = new QWidget(l2);
      auto l4 = new QWidget(l3);
      {
        auto selection = select(selector, {&find_stylist(*l4)});
        REQUIRE(selection.size() == 4);
        REQUIRE(selection.contains(&find_stylist(*l3)));
        REQUIRE(selection.contains(&find_stylist(*l2)));
        REQUIRE(selection.contains(&find_stylist(*l1)));
        REQUIRE(selection.contains(&find_stylist(*root)));
      }
      {
        auto selection = select(selector, {&find_stylist(*l3)});
        REQUIRE(selection.size() == 3);
        REQUIRE(selection.contains(&find_stylist(*l2)));
        REQUIRE(selection.contains(&find_stylist(*l1)));
        REQUIRE(selection.contains(&find_stylist(*root)));
      }
      {
        auto selection = select(selector, {&find_stylist(*l2)});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(*l1)));
        REQUIRE(selection.contains(&find_stylist(*root)));
      }
      {
        auto selection = select(selector, {&find_stylist(*l1)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(*root)));
      }
      {
        auto selection = select(selector, {&find_stylist(*root)});
        REQUIRE(selection.empty());
      }
    });
  }
}
