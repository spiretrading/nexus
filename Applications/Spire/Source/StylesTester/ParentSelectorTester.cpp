#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("ParentSelector") {
  TEST_CASE("equality") {
    REQUIRE(ParentSelector(Foo(), Foo()) == ParentSelector(Foo(), Foo()));
    REQUIRE(ParentSelector(Foo(), Foo()) != ParentSelector(Foo(), Bar()));
    REQUIRE(ParentSelector(Foo(), Foo()) != ParentSelector(Bar(), Foo()));
    REQUIRE(ParentSelector(Foo(), Bar()) != ParentSelector(Foo(), Foo()));
    REQUIRE(ParentSelector(Bar(), Foo()) != ParentSelector(Foo(), Foo()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(ParentSelector(Foo(), Bar()),
        find_stylist(*graph["D"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["D"], Foo());
      REQUIRE(updates.empty());
      match(*graph["B"], Bar());
      require_selection(updates, graph, {"B"}, {});
      match(*graph["A"], Bar());
      REQUIRE(updates.empty());
      graph["D"]->setParent(graph["A"]);
      require_selection(updates, graph, {"A"}, {"B"});
      match(*graph["F"], Bar());
      REQUIRE(updates.empty());
      graph["D"]->setParent(graph["F"]);
      require_selection(updates, graph, {"F"}, {"A"});
      graph["D"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"F"});
    });
  }

  TEST_CASE("initial_selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(ParentSelector(Any(), Any()),
        find_stylist(*graph["D"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      require_selection(updates, graph, {"B"}, {});
    });
  }
}
