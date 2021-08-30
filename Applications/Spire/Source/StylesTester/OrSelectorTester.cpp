#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

TEST_SUITE("OrSelector") {
  TEST_CASE("equality") {
    REQUIRE(OrSelector(Any(), Any()) == OrSelector(Any(), Any()));
    REQUIRE(OrSelector(Any(), Any()) != OrSelector(Any(), Hover()));
    REQUIRE(OrSelector(Any(), Any()) != OrSelector(Hover(), Any()));
    REQUIRE(OrSelector(Any(), Hover()) != OrSelector(Any(), Any()));
    REQUIRE(OrSelector(Hover(), Any()) != OrSelector(Any(), Any()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto selector = OrSelector(Hover(), Focus());
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
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w1)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w1)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Hover());
        match(w1, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w1)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w2, Hover());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w2, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 1);
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
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
        match(w2, Hover());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(w1)));
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Focus());
        match(w2, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(w1)));
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Hover());
        match(w2, Focus());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(w1)));
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
      {
        auto w1 = QWidget();
        auto w2 = QWidget();
        match(w1, Focus());
        match(w2, Hover());
        auto selection =
          select(selector, {&find_stylist(w1), &find_stylist(w2)});
        REQUIRE(selection.size() == 2);
        REQUIRE(selection.contains(&find_stylist(w1)));
        REQUIRE(selection.contains(&find_stylist(w2)));
      }
    });
  }
}
