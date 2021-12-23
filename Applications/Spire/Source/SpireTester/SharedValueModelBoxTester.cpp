#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/SharedValueModelBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("SharedValueModelBox") {
  TEST_CASE("state") {
    auto value = std::make_shared<int>(123);
    auto model = std::make_shared<SharedValueModel<int>>(value);
    auto box = SharedValueModelBox(model);
    auto updates = std::deque<bool>();
    auto connection = scoped_connection(box.connect_update_signal([&] () {
      updates.push_back(true);
    }));
    model->set(555);
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == true);
    updates.pop_front();
    box.signal_update();
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == true);
    updates.pop_front();
  }
}
