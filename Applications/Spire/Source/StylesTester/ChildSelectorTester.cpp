#include <doctest/doctest.h>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("ChildSelector") {
  TEST_CASE("equality") {
    REQUIRE(ChildSelector(Foo(), Foo()) == ChildSelector(Foo(), Foo()));
    REQUIRE(ChildSelector(Foo(), Foo()) != ChildSelector(Foo(), Bar()));
    REQUIRE(ChildSelector(Foo(), Foo()) != ChildSelector(Bar(), Foo()));
    REQUIRE(ChildSelector(Foo(), Bar()) != ChildSelector(Foo(), Foo()));
    REQUIRE(ChildSelector(Bar(), Foo()) != ChildSelector(Foo(), Foo()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(ChildSelector(Foo(), Bar()),
        find_stylist(*graph["B"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["B"], Foo());
      REQUIRE(updates.empty());
      match(*graph["D"], Bar());
      require_selection(updates, graph, {"D"}, {});
      match(*graph["C"], Bar());
      require_selection(updates, graph, {"C"}, {});
      graph["D"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"D"});
      graph["C"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"C"});
      auto child = QWidget();
      graph["Z"] = &child;
      match(child, Bar());
      REQUIRE(updates.empty());
      child.setParent(graph["B"]);
      require_selection(updates, graph, {"Z"}, {});
    });
  }

  TEST_CASE("initial_selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(ChildSelector(Any(), Any()),
        find_stylist(*graph["B"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      require_selection(updates, graph, {"C", "D"}, {});
    });
  }

  TEST_CASE("recycle") {
    run_test([] {
      auto parent = QWidget();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(ChildSelector(Any(), Any()),
        find_stylist(parent), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      auto child = optional<QWidget>();
      child.emplace();
      child->setParent(&parent);
      REQUIRE(updates.size() == 1);
      auto update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_additions.size() == 1);
      REQUIRE(update.m_additions.contains(&find_stylist(*child)));
      REQUIRE(update.m_removals.empty());
      auto child_stylist = &find_stylist(*child);
      child = none;
      REQUIRE(updates.size() == 1);
      update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_removals.size() == 1);
      REQUIRE(update.m_removals.contains(child_stylist));
      REQUIRE(update.m_additions.empty());
      child.emplace();
      child->setParent(&parent);
      REQUIRE(updates.size() == 1);
      update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_additions.size() == 1);
      REQUIRE(update.m_additions.contains(&find_stylist(*child)));
      REQUIRE(update.m_removals.empty());
    });
  }

  TEST_CASE("flip_compound_child") {
    run_test([] {
      auto parent = QWidget();
      auto updates_a = std::deque<SelectionUpdate>();
      auto row_selector = ChildSelector(+Any(), ChildSelector(Any(), Foo()));
      auto connection_a = select(row_selector, find_stylist(parent),
        [&] (auto&& additions, auto&& removals) {
          updates_a.push_back({std::move(additions), std::move(removals)});
        });
      auto updates_b = std::deque<SelectionUpdate>();
      auto connection_b =
        select(ChildSelector(row_selector, ChildSelector(Any(), Any())),
          find_stylist(parent), [&] (auto&& additions, auto&& removals) {
            updates_b.push_back({std::move(additions), std::move(removals)});
          });
      REQUIRE(updates_a.empty());
      auto child_a = new QWidget();
      child_a->setParent(&parent);
      REQUIRE(updates_a.empty());
      for(auto i = 0; i != 100; ++i) {
        auto child_b = new QWidget();
        match(*child_b, Foo());
        child_b->setParent(&*child_a);
        require_selection(updates_a, {&find_stylist(parent)}, {});
        require_selection(updates_b, {&find_stylist(*child_b)}, {});
        auto child_b_stylist = &find_stylist(*child_b);
        child_b->setParent(nullptr);
        unmatch(*child_b, Foo());
        delete child_b;
        require_selection(updates_a, {}, {&find_stylist(parent)});
        require_selection(updates_b, {}, {child_b_stylist});
      }
    });
  }
}
