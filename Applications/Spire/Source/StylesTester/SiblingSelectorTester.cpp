#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("SiblingSelector") {
  TEST_CASE("equality") {
    REQUIRE(SiblingSelector(Foo(), Foo()) == SiblingSelector(Foo(), Foo()));
    REQUIRE(SiblingSelector(Foo(), Foo()) != SiblingSelector(Foo(), Bar()));
    REQUIRE(SiblingSelector(Foo(), Foo()) != SiblingSelector(Bar(), Foo()));
    REQUIRE(SiblingSelector(Foo(), Bar()) != SiblingSelector(Foo(), Foo()));
    REQUIRE(SiblingSelector(Bar(), Foo()) != SiblingSelector(Foo(), Foo()));
  }

  TEST_CASE("selection") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(SiblingSelector(Foo(), Bar()),
        find_stylist(*graph["C"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["C"], Foo());
      REQUIRE(updates.empty());
      match(*graph["D"], Bar());
      require_selection(updates, graph, {"D"}, {});
      auto sibling = QWidget();
      graph["Z"] = &sibling;
      sibling.setParent(graph["B"]);
      REQUIRE(updates.empty());
      match(sibling, Bar());
      require_selection(updates, graph, {"Z"}, {});
      match(*graph["G"], Bar());
      match(*graph["H"], Bar());
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
