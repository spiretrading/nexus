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
      auto updates = std::deque<SelectionUpdate>();
      auto connection =
        select(ChildSelector(Any(), ChildSelector(+Foo(), Bar())),
          find_stylist(parent), [&] (auto&& additions, auto&& removals) {
            updates.push_back({std::move(additions), std::move(removals)});
          });
      REQUIRE(updates.empty());
      auto child_a = optional<QWidget>();
      child_a.emplace();
      match(*child_a, Foo());
      child_a->setParent(&parent);
      REQUIRE(updates.empty());
      auto child_b = optional<QWidget>();
      child_b.emplace();
      match(*child_b, Bar());
      child_b->setParent(&*child_a);
      REQUIRE(updates.size() == 1);
      auto update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_additions.size() == 1);
      REQUIRE(update.m_additions.contains(&find_stylist(*child_a)));
      REQUIRE(update.m_removals.empty());
      child_b = none;
      REQUIRE(updates.size() == 1);
      update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_removals.size() == 1);
      REQUIRE(update.m_removals.contains(&find_stylist(*child_a)));
      REQUIRE(update.m_additions.empty());
      child_b.emplace();
      match(*child_b, Bar());
      child_b->setParent(&*child_a);
      REQUIRE(updates.size() == 1);
      update = updates.front();
      updates.pop_front();
      REQUIRE(update.m_additions.size() == 1);
      REQUIRE(update.m_additions.contains(&find_stylist(*child_a)));
      REQUIRE(update.m_removals.empty());
    });
  }
}
