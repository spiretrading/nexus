#include <atomic>
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
    auto updates = std::make_shared<std::atomic_int>(0);
    model->connect_update_signal([=] (const auto& value) {
      ++*updates;
    });
    auto state = std::make_shared<std::promise<QValidator::State>>();
    auto state_result = state->get_future();
    auto setter = std::thread([=] {
      state->set_value(model->set(123));
    });
    setter.detach();
    REQUIRE(state_result.wait_for(std::chrono::seconds(5)) ==
      std::future_status::ready);
    REQUIRE(state_result.get() == QValidator::State::Invalid);
    REQUIRE(model->get() == 0);
    REQUIRE(*updates == 0);
  }
}
