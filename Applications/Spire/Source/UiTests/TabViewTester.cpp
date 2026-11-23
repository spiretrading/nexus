#include <boost/optional/optional_io.hpp>
#include "Spire/Ui/TabView.hpp"
#include "Spire/UiTests/UiTests.hpp"

using namespace Spire;

TEST_SUITE("TabView") {
  UI_TEST_CASE("current_tab") {
    auto view = TabView();
    auto first = new QWidget();
    auto second = new QWidget();
    REQUIRE(!view.get_current()->get());
    view.add("First", *first);
    view.add("Second", *second);
    view.show();
    REQUIRE(view.get_current()->get() == 0);
    REQUIRE(first->isVisible());
    REQUIRE_FALSE(second->isVisible());
    view.get_current()->set(1);
    REQUIRE(view.get_current()->get() == 1);
    REQUIRE_FALSE(first->isVisible());
    REQUIRE(second->isVisible());
    REQUIRE(view.get_current()->set(2) == QValidator::Invalid);
    REQUIRE(view.get_current()->get() == 1);
  }
}
