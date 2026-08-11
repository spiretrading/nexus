#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <doctest/doctest.h>
#include "Spire/Spire/QtValueModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_SUITE("QtValueModel") {
  TEST_CASE("set_from_another_thread") {
    run_test([] {
      auto model = QtValueModel<int>(0);
      auto setter = std::thread([&] {
        model.set(123);
      });
      while(model.get() != 123) {
        QApplication::processEvents(QEventLoop::WaitForMoreEvents);
      }
      setter.join();
      REQUIRE(model.get() == 123);
    });
  }

  TEST_CASE("set_after_event_loop_stops") {
    auto model = std::shared_ptr<QtValueModel<int>>();
    run_test([&] {
      model = std::make_shared<QtValueModel<int>>(0);
    });
    auto is_set = std::make_shared<std::promise<void>>();
    auto is_set_result = is_set->get_future();
    auto setter = std::thread([=] {
      model->set(123);
      is_set->set_value();
    });
    setter.detach();
    REQUIRE(is_set_result.wait_for(std::chrono::seconds(5)) ==
      std::future_status::ready);
    REQUIRE(model->get() == 123);
  }
}
