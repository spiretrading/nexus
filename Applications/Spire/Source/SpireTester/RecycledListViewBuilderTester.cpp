#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/RecycledListViewBuilder.hpp"

using namespace Spire;

TEST_SUITE("RecycledListViewBuilder") {
  TEST_CASE("recycle") {
    run_test([] {
      auto builder = RecycledListViewBuilder<ListModel<int>>();
      auto model = std::make_shared<ArrayListModel<int>>();
      auto w1 = builder.mount(model, 0);
      builder.unmount(w1, 0);
      auto w2 = builder.mount(model, 0);
      REQUIRE(w1 == w2);
    });
  }
}
