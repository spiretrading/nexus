#include <deque>
#include <doctest/doctest.h>
#include "Spire/Spire/CompositeModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Spire;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };
}

TEST_SUITE("CompositeModel") {
  TEST_CASE("add") {
    auto x = std::make_shared<LocalValueModel<int>>();
    x->set(553);
    auto y = std::make_shared<LocalValueModel<int>>();
    y->set(442);
    auto model = std::make_shared<CompositeModel<Point>>();
    auto updates = std::deque<Point>();
    model->connect_update_signal([&] (Point current) {
      updates.push_back(current);
    });
    model->add(&Point::m_x, x);
    REQUIRE(updates.size() == 1);
    auto update = updates.front();
    updates.pop_front();
    REQUIRE(update.m_x == 553);
    REQUIRE(model->get().m_x == 553);
    model->add(&Point::m_y, y);
    REQUIRE(updates.size() == 1);
    update = updates.front();
    updates.pop_front();
    REQUIRE(update.m_y == 442);
    REQUIRE(model->get().m_y == 442);
  }

  TEST_CASE("access") {
    auto x = std::make_shared<LocalValueModel<int>>();
    auto x_updates = std::deque<int>();
    x->connect_update_signal([&] (int current) {
      x_updates.push_back(current);
    });
    auto y = std::make_shared<LocalValueModel<int>>();
    auto y_updates = std::deque<int>();
    y->connect_update_signal([&] (int current) {
      y_updates.push_back(current);
    });
    auto model = std::make_shared<CompositeModel<Point>>();
    model->add(&Point::m_x, x);
    model->add(&Point::m_y, y);
    auto updates = std::deque<Point>();
    model->connect_update_signal([&] (Point current) {
      updates.push_back(current);
    });
    x->set(123);
    x_updates.pop_front();
    REQUIRE(updates.size() == 1);
    auto update = updates.front();
    updates.pop_front();
    REQUIRE(update.m_x == 123);
    REQUIRE(model->get().m_x == 123);
    y->set(321);
    y_updates.pop_front();
    REQUIRE(updates.size() == 1);
    update = updates.front();
    updates.pop_front();
    REQUIRE(update.m_y == 321);
    REQUIRE(model->get().m_y == 321);
    model->set(Point(12, 19));
    REQUIRE(x_updates.size() == 1);
    auto x_update = x_updates.front();
    x_updates.pop_front();
    REQUIRE(x_update == 12);
    REQUIRE(y_updates.size() == 1);
    auto y_update = y_updates.front();
    y_updates.pop_front();
    REQUIRE(y_update == 19);
    REQUIRE(x->get() == 12);
    REQUIRE(y->get() == 19);
  }
}
