#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

namespace {
  void require_addition(std::deque<SelectionUpdate>& updates, QWidget& child) {
    REQUIRE(updates.size() == 1);
    auto update = updates.front();
    updates.pop_front();
    REQUIRE(update.m_additions.size() == 1);
    REQUIRE(update.m_additions.count(&find_stylist(child)) == 1);
    REQUIRE(update.m_removals.empty());
  }

  void require_removal(std::deque<SelectionUpdate>& updates, QWidget& child) {
    REQUIRE(updates.size() == 1);
    auto update = updates.front();
    updates.pop_front();
    REQUIRE(update.m_removals.size() == 1);
    REQUIRE(update.m_removals.count(&find_stylist(child)) == 1);
    REQUIRE(update.m_additions.empty());
  }
}

TEST_SUITE("TopSelector") {
  TEST_CASE("equality") {
    REQUIRE(TopSelector(Foo(), Foo()) == TopSelector(Foo(), Foo()));
    REQUIRE(TopSelector(Foo(), Foo()) != TopSelector(Foo(), Bar()));
    REQUIRE(TopSelector(Foo(), Foo()) != TopSelector(Bar(), Foo()));
    REQUIRE(TopSelector(Foo(), Bar()) != TopSelector(Foo(), Foo()));
    REQUIRE(TopSelector(Bar(), Foo()) != TopSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(TopSelector(Foo(), Bar()),
        find_stylist(*graph["A"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["A"], Foo());
      REQUIRE(updates.empty());
      match(*graph["C"], Bar());
      require_selection(updates, graph, {"C"}, {});
      match(*graph["H"], Bar());
      require_selection(updates, graph, {"H"}, {});
      match(*graph["F"], Bar());
      REQUIRE(updates.empty());
      match(*graph["E"], Bar());
      REQUIRE(updates.empty());
      unmatch(*graph["C"], Bar());
      require_selection(updates, graph, {"E", "F"}, {"C"});
    });
  }

  TEST_CASE("combinator") {
    run_test([] {
      auto root = new QWidget();
      root->setObjectName("Root");
      auto child1 = new QWidget(root);
      child1->setObjectName("Child1");
      auto child2 = new QWidget(root);
      child2->setObjectName("Child2");
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(TopSelector(Foo(), Bar()),
        find_stylist(*root), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      match(*child1, Bar());
      REQUIRE(updates.empty());
      match(*root, Foo());
      require_addition(updates, *child1);
      unmatch(*child1, Bar());
      require_removal(updates, *child1);
      match(*child2, Bar());
      require_addition(updates, *child2);
      unmatch(*root, Foo());
      require_removal(updates, *child2);
      match(*root, Foo());
      require_addition(updates, *child2);
      unmatch(*child2, Bar());
      require_removal(updates, *child2);
      match(*child1, Bar());
      require_addition(updates, *child1);
    });
  }
}
