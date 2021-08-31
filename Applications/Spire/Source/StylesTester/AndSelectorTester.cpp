#include <deque>
#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct Update {
    std::unordered_set<const Stylist*> m_additions;
    std::unordered_set<const Stylist*> m_removals;
  };
}

TEST_SUITE("AndSelector") {
  TEST_CASE("equality") {
    REQUIRE(AndSelector(Any(), Any()) == AndSelector(Any(), Any()));
    REQUIRE(AndSelector(Any(), Any()) != AndSelector(Any(), Hover()));
    REQUIRE(AndSelector(Any(), Any()) != AndSelector(Hover(), Any()));
    REQUIRE(AndSelector(Any(), Hover()) != AndSelector(Any(), Any()));
    REQUIRE(AndSelector(Hover(), Any()) != AndSelector(Any(), Any()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto w1 = QWidget();
      auto updates = std::deque<Update>();
      auto connection = select(AndSelector(Hover(), Focus()), find_stylist(w1),
        [&] (const auto& additions, const auto& removals) {
          updates.push_back({additions, removals});
        });
      REQUIRE(updates.empty());
      match(w1, Hover());
      REQUIRE(updates.empty());
      match(w1, Focus());
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
      REQUIRE(updates.size() == 1);
      {
        auto matches = updates.front();
        updates.pop_front();
        REQUIRE(matches.m_additions.empty());
        REQUIRE(matches.m_removals.size() == 1);
        REQUIRE(matches.m_removals.contains(&find_stylist(w1)));
      }
      unmatch(w1, Focus());
      REQUIRE(updates.empty());
    });
  }

  TEST_CASE("select_initialization") {
    run_test([] {
      auto w1 = QWidget();
      match(w1, Hover());
      match(w1, Focus());
      auto updates = std::deque<Update>();
      auto connection = select(AndSelector(Hover(), Focus()), find_stylist(w1),
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
