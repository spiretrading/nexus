#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("AndSelector") {
  TEST_CASE("equality") {
    REQUIRE(AndSelector(Any(), Any()) == AndSelector(Any(), Any()));
    REQUIRE(AndSelector(Any(), Any()) != AndSelector(Any(), Hover()));
    REQUIRE(AndSelector(Any(), Any()) != AndSelector(Hover(), Any()));
    REQUIRE(AndSelector(Any(), Hover()) != AndSelector(Any(), Any()));
    REQUIRE(AndSelector(Hover(), Any()) != AndSelector(Any(), Any()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto selector = AndSelector(Hover(), Focus());
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.empty());
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Hover());
        match(w2, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.empty());
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Focus());
        match(w2, Hover());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.empty());
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Hover());
        match(w1, Focus());
        match(w2, Hover());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w1)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Hover());
        match(w2, Hover());
        match(w2, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Hover());
        match(w1, Focus());
        match(w2, Hover());
        match(w2, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(w1)));
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
    });
  }
}
