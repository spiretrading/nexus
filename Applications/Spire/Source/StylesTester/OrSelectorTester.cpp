#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("OrSelector") {
  TEST_CASE("equality") {
    REQUIRE(OrSelector(Any(), Any()) == OrSelector(Any(), Any()));
    REQUIRE(OrSelector(Any(), Any()) != OrSelector(Any(), Hover()));
    REQUIRE(OrSelector(Any(), Any()) != OrSelector(Hover(), Any()));
    REQUIRE(OrSelector(Any(), Hover()) != OrSelector(Any(), Any()));
    REQUIRE(OrSelector(Hover(), Any()) != OrSelector(Any(), Any()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(OrSelector(Hover(), Focus()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      match(w1, Hover());
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.size() == 1);
        REQUIRE(matches.m_removals.empty());
        REQUIRE(matches.m_additions.contains(&find_stylist(w1)));
      }
      match(w1, Focus());
      REQUIRE(updates.empty());
      unmatch(w1, Hover());
      unmatch(w1, Focus());
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
      match(w1, Hover());
      match(w1, Focus());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(OrSelector(Hover(), Focus()), find_stylist(w1),
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
