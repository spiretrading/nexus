#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("Any") {
  TEST_CASE("equality") {
    REQUIRE(Any() == Any());
    REQUIRE(!(Any() != Any()));
  }

  TEST_CASE("empty_selection") {
    run_test([] {
      auto selection = select(Any(), {});
      REQUIRE(selection.empty());
    });
  }

  TEST_CASE("single_selection") {
    run_test([] {
      auto widget = QWidget();
      auto& stylist = find_stylist(widget);
      auto selection = select(Any(), {&stylist});
      REQUIRE(selection.size() == 1);
      REQUIRE(selection.contains(&stylist));
    });
  }

  TEST_CASE("multiple_selection") {
    run_test([] {
      auto widget1 = QWidget();
      auto widget2 = QWidget();
      auto& stylist1 = find_stylist(widget1);
      auto& stylist2 = find_stylist(widget2);
      auto selection = select(Any(), {&stylist1, &stylist2});
      REQUIRE(selection.contains(&stylist1));
      REQUIRE(selection.contains(&stylist2));
    });
  }
}
