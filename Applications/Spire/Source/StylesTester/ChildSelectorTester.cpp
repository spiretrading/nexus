#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("ChildSelector") {
  TEST_CASE("equality") {
    REQUIRE(ChildSelector(Any(), Any()) == ChildSelector(Any(), Any()));
    REQUIRE(ChildSelector(Any(), Any()) != ChildSelector(Any(), Hover()));
    REQUIRE(ChildSelector(Any(), Any()) != ChildSelector(Hover(), Any()));
    REQUIRE(ChildSelector(Any(), Hover()) != ChildSelector(Any(), Any()));
    REQUIRE(ChildSelector(Hover(), Any()) != ChildSelector(Any(), Any()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(ChildSelector(Hover(), Focus()),
        find_stylist(*graph["B"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["B"], Hover());
      REQUIRE(updates.empty());
      match(*graph["D"], Focus());
      require_selection(updates, graph, {"D"}, {});
      match(*graph["C"], Focus());
      require_selection(updates, graph, {"C"}, {});
      graph["D"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"D"});
      graph["C"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"C"});
      auto child = QWidget();
      graph["Z"] = &child;
      match(child, Focus());
      REQUIRE(updates.empty());
      child.setParent(graph["B"]);
      require_selection(updates, graph, {"Z"}, {});
    });
  }

  TEST_CASE("selection") {
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
