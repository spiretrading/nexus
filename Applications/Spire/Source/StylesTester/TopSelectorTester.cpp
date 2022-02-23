#include <doctest/doctest.h>
#include <QWidget>
#include "Spire/Styles/Selectors.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("TopSelector") {
  TEST_CASE("equality") {
    REQUIRE(TopSelector(Foo(), Foo()) == TopSelector(Foo(), Foo()));
    REQUIRE(TopSelector(Foo(), Foo()) != TopSelector(Foo(), Bar()));
    REQUIRE(TopSelector(Foo(), Foo()) != TopSelector(Bar(), Foo()));
    REQUIRE(TopSelector(Foo(), Bar()) != TopSelector(Foo(), Foo()));
    REQUIRE(TopSelector(Bar(), Foo()) != TopSelector(Foo(), Foo()));
  }

  TEST_CASE("select") {
    run_test([] {
      auto graph = make_graph();
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(TopSelector(Foo(), Bar()),
        find_stylist(*graph["A"]), [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      REQUIRE(updates.empty());
      match(*graph["A"], Foo());
      REQUIRE(updates.empty());
      match(*graph["C"], Bar());
      require_selection(updates, graph, {"C"}, {});
      match(*graph["H"], Bar());
      require_selection(updates, graph, {"H"}, {});
      match(*graph["F"], Bar());
      REQUIRE(updates.empty());
      match(*graph["E"], Bar());
      REQUIRE(updates.empty());
      unmatch(*graph["C"], Bar());
      require_selection(updates, graph, {"E", "F"}, {"C"});
    });
  }
}
