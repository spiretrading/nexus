#include <doctest/doctest.h>
#include "Spire/Styles/CombinatorSelector.hpp"
#include "Spire/StylesTester/StylesTester.hpp"

using namespace Spire;
using namespace Spire::Styles;
using namespace Spire::Styles::Tests;

TEST_SUITE("CombinatorSelector") {
  TEST_CASE("evaluate") {
    run_test([] {
      auto graph = make_graph();
      match(*graph["A"], Foo());
      match(*graph["B"], Bar());
      match(*graph["C"], Bar());
      auto updates = std::deque<SelectionUpdate>();
      auto connection = select(CombinatorSelector(Foo(), Bar(),
        [&] (const auto& stylist, const auto& on_update) {
          on_update(
            {&find_stylist(*graph["B"]), &find_stylist(*graph["C"])}, {});
          return SelectConnection();
        }), find_stylist(*graph["A"]),
        [&] (auto&& additions, auto&& removals) {
          updates.push_back({std::move(additions), std::move(removals)});
        });
      delete graph["C"];
      unmatch(*graph["B"], Bar());
    });
  }
}
