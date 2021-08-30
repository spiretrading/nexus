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
}

TEST_SUITE("StateSelector") {
  TEST_CASE("equality") {
    REQUIRE(Foo() == Foo());
    REQUIRE(Baz(0) == Baz(0));
    REQUIRE(Baz(0) != Baz(1));
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
