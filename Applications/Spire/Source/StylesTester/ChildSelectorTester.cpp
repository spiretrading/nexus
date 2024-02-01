#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

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
}
