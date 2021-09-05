#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("DescendantSelector") {
  TEST_CASE("equality") {
    REQUIRE(
      DescendantSelector(Any(), Any()) == DescendantSelector(Any(), Any()));
    REQUIRE(
      DescendantSelector(Any(), Any()) != DescendantSelector(Any(), Hover()));
    REQUIRE(
      DescendantSelector(Any(), Any()) != DescendantSelector(Hover(), Any()));
    REQUIRE(
      DescendantSelector(Any(), Hover()) != DescendantSelector(Any(), Any()));
    REQUIRE(
      DescendantSelector(Hover(), Any()) != DescendantSelector(Any(), Any()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(DescendantSelector(Hover(), Focus()),
        find_stylist(*graph["B"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["B"], Hover());
      REQUIRE(updates.empty());
      match(*graph["C"], Focus());
      require_selection(updates, graph, {"C"}, {});
      match(*graph["H"], Focus());
      require_selection(updates, graph, {"H"}, {});
      graph["D"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"H"});
    });
  }
}
