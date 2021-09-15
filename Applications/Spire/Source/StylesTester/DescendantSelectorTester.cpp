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
      DescendantSelector(Foo(), Foo()) == DescendantSelector(Foo(), Foo()));
    REQUIRE(
      DescendantSelector(Foo(), Foo()) != DescendantSelector(Foo(), Bar()));
    REQUIRE(
      DescendantSelector(Foo(), Foo()) != DescendantSelector(Bar(), Foo()));
    REQUIRE(
      DescendantSelector(Foo(), Bar()) != DescendantSelector(Foo(), Foo()));
    REQUIRE(
      DescendantSelector(Bar(), Foo()) != DescendantSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(DescendantSelector(Foo(), Bar()),
        find_stylist(*graph["B"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["B"], Foo());
      REQUIRE(updates.empty());
      match(*graph["C"], Bar());
      require_selection(updates, graph, {"C"}, {});
      match(*graph["H"], Bar());
      require_selection(updates, graph, {"H"}, {});
      graph["D"]->setParent(nullptr);
      require_selection(updates, graph, {}, {"H"});
    });
  }
}
