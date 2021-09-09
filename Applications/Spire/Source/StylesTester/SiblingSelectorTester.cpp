#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("SiblingSelector") {
  TEST_CASE("equality") {
    REQUIRE(SiblingSelector(Any(), Any()) == SiblingSelector(Any(), Any()));
    REQUIRE(SiblingSelector(Any(), Any()) != SiblingSelector(Any(), Hover()));
    REQUIRE(SiblingSelector(Any(), Any()) != SiblingSelector(Hover(), Any()));
    REQUIRE(SiblingSelector(Any(), Hover()) != SiblingSelector(Any(), Any()));
    REQUIRE(SiblingSelector(Hover(), Any()) != SiblingSelector(Any(), Any()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(SiblingSelector(Hover(), Focus()),
        find_stylist(*graph["C"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["C"], Hover());
      REQUIRE(updates.empty());
      match(*graph["D"], Focus());
      require_selection(updates, graph, {"D"}, {});
      auto sibling = QWidget();
      graph["Z"] = &sibling;
      sibling.setParent(graph["B"]);
      REQUIRE(updates.empty());
      match(sibling, Focus());
      require_selection(updates, graph, {"Z"}, {});
      match(*graph["G"], Focus());
      match(*graph["H"], Focus());
      REQUIRE(updates.empty());
      graph["C"]->setParent(graph["D"]);
      require_selection(updates, graph, {"G", "H"}, {"D", "Z"});
      graph["C"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"G", "H"});
    });
  }

  TEST_CASE("initial_selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(SiblingSelector(Any(), Any()),
        find_stylist(*graph["C"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      require_selection(updates, graph, {"D"}, {});
    });
  }
}
