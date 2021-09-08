#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("ParentSelector") {
  TEST_CASE("equality") {
    REQUIRE(ParentSelector(Any(), Any()) == ParentSelector(Any(), Any()));
    REQUIRE(ParentSelector(Any(), Any()) != ParentSelector(Any(), Hover()));
    REQUIRE(ParentSelector(Any(), Any()) != ParentSelector(Hover(), Any()));
    REQUIRE(ParentSelector(Any(), Hover()) != ParentSelector(Any(), Any()));
    REQUIRE(ParentSelector(Hover(), Any()) != ParentSelector(Any(), Any()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(ParentSelector(Hover(), Focus()),
        find_stylist(*graph["D"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["D"], Hover());
      REQUIRE(updates.empty());
      match(*graph["B"], Focus());
      require_selection(updates, graph, {"B"}, {});
      match(*graph["A"], Focus());
      REQUIRE(updates.empty());
      graph["D"]->setParent(graph["A"]);
      require_selection(updates, graph, {"A"}, {"B"});
      match(*graph["F"], Focus());
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
