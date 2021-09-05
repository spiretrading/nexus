#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("AncestorSelector") {
  TEST_CASE("equality") {
    REQUIRE(AncestorSelector(Any(), Any()) == AncestorSelector(Any(), Any()));
    REQUIRE(AncestorSelector(Any(), Any()) != AncestorSelector(Any(), Hover()));
    REQUIRE(AncestorSelector(Any(), Any()) != AncestorSelector(Hover(), Any()));
    REQUIRE(AncestorSelector(Any(), Hover()) != AncestorSelector(Any(), Any()));
    REQUIRE(AncestorSelector(Hover(), Any()) != AncestorSelector(Any(), Any()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AncestorSelector(Hover(), Focus()),
        find_stylist(*graph["G"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["G"], Hover());
      REQUIRE(updates.empty());
      match(*graph["D"], Focus());
      require_selection(updates, graph, {"D"}, {});
      match(*graph["A"], Focus());
      require_selection(updates, graph, {"A"}, {});
      match(*graph["B"], Focus());
      require_selection(updates, graph, {"B"}, {});
      match(*graph["C"], Focus());
      graph["G"]->setParent(graph["C"]);
      require_selection(updates, graph, {"C"}, {"D"});
      graph["G"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"C", "B", "A"});
    });
  }

  TEST_CASE("initial_selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AncestorSelector(Any(), Any()),
        find_stylist(*graph["G"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      require_selection(updates, graph, {"D", "B", "A"}, {});
    });
  }
}
