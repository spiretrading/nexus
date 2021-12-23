#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/ReferenceValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ReferenceValueModel") {
  TEST_CASE("state") {
    auto value = 123;
    auto model = std::make_shared<ReferenceValueModel<int>>(value);
    auto updates = std::deque<int>();
    auto connection = scoped_connection(model->connect_update_signal(
      [&] (auto value) {
        updates.push_back(value);
      }));
    REQUIRE(model->get() == 123);
    value = 321;
    REQUIRE(model->get() == 321);
    REQUIRE(model->set(555) == QValidator::State::Acceptable);
    REQUIRE(value == 555);
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == 555);
    updates.pop_front();
    value = 222;
    model->signal_update();
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == 222);
    updates.pop_front();
  }
}
