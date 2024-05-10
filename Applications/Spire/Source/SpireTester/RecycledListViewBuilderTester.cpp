#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/RecycledListViewBuilder.hpp"

using namespace Spire;

namespace {
  struct TestViewBuilder {
    QWidget* mount(const std::shared_ptr<ListModel<int>>& model, int index) {
      return new QWidget();
    }

    void reset(QWidget& widget, const std::shared_ptr<ListModel<int>>& model,
      int index) {}

    void unmount(QWidget* widget) {
      delete widget;
    }
  };
}

TEST_SUITE("RecycledListViewBuilder") {
  TEST_CASE("recycle") {
    run_test([] {
      auto builder = RecycledListViewBuilder(TestViewBuilder());
      auto model = std::make_shared<ArrayListModel<int>>();
      model->push(1);
      auto w1 = builder.mount(model, 0);
      builder.unmount(w1, 0);
      auto w2 = builder.mount(model, 0);
      REQUIRE(w1 == w2);
    });
  }
}
