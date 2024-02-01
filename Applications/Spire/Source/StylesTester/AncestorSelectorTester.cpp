#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("AncestorSelector") {
  TEST_CASE("equality") {
    REQUIRE(AncestorSelector(Foo(), Foo()) == AncestorSelector(Foo(), Foo()));
    REQUIRE(AncestorSelector(Foo(), Foo()) != AncestorSelector(Foo(), Bar()));
    REQUIRE(AncestorSelector(Foo(), Foo()) != AncestorSelector(Bar(), Foo()));
    REQUIRE(AncestorSelector(Foo(), Bar()) != AncestorSelector(Foo(), Foo()));
    REQUIRE(AncestorSelector(Bar(), Foo()) != AncestorSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(AncestorSelector(Foo(), Bar()),
        find_stylist(*graph["G"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["G"], Foo());
      REQUIRE(updates.empty());
      match(*graph["D"], Bar());
      require_selection(updates, graph, {"D"}, {});
      match(*graph["A"], Bar());
      require_selection(updates, graph, {"A"}, {});
      match(*graph["B"], Bar());
      require_selection(updates, graph, {"B"}, {});
      match(*graph["C"], Bar());
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
