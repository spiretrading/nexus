#include <doctest/doctest.h>
#include "Spire/Async/QtFuture.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto CURRENT_TIME =
    ptime(date(2024, Jun, 01), time_duration(10, 15, 0));
  const auto LOAD_COUNT = 2;

  std::time_t to_time_t_milliseconds(ptime pt) {
    return (pt - ptime(date(1970, 1, 1))).total_milliseconds();
  }

  void run_test_two_loads(TimeAndSalesTableModel& model,
      int first_load_count, int second_load_count,
      std::function<void ()> first_loading_test,
      std::function<void ()> after_first_load_test,
      std::function<void()> second_loading_test,
      std::function<void ()> after_second_load_test) {
    run_test([&] {
      REQUIRE(model.get_row_size() == 0);
      auto count = std::make_shared<int>(0);
      auto future_promises =
        std::array<std::pair<QtFuture<void>, QtPromise<void>>, LOAD_COUNT>{
          make_future<void>(), make_future<void>()};
      auto futures = std::array<std::shared_ptr<QtFuture<void>>, LOAD_COUNT>{
        std::make_shared<QtFuture<void>>(std::move(future_promises[0].first)),
          std::make_shared<QtFuture<void>>(
            std::move(future_promises[1].first))};
      model.connect_end_loading_signal([=] {
        if(*count == 0) {
          first_loading_test();
        } else {
          second_loading_test();
        }
        futures[*count]->resolve();
      });
      model.load_history(first_load_count);
      wait(std::move(future_promises[*count].second));
      after_first_load_test();
      ++*count;
      model.load_history(second_load_count);
      wait(std::move(future_promises[*count].second));
      after_second_load_test();
    });
  }

  void test_model(const TimeAndSalesTableModel& model) {
    for(auto i = 0; i < model.get_row_size(); ++i) {
      REQUIRE(model.get<ptime>(i, 0) == CURRENT_TIME - seconds(i));
    }
  }

  TimeAndSalesModel::Entry make_entry(ptime timestamp) {
    return TimeAndSalesModel::Entry{SequencedValue(
      TimeAndSale(timestamp, Money::ONE, 100, TimeAndSale::Condition(
        TimeAndSale::Condition::Type::REGULAR, "@"), "XNYS"),
      Beam::Sequence(to_time_t_milliseconds(timestamp))),
      BboIndicator::UNKNOWN};
  }

  struct TestTimeAndSalesModel : TimeAndSalesModel {
    mutable UpdateSignal m_update_signal;
    std::vector<Entry> m_historical_data;
    ptime m_current_time;
    int m_offset;

    TestTimeAndSalesModel()
        : m_current_time(CURRENT_TIME),
          m_offset(0) {
      for(auto i = 0; i < 10; ++i) {
        m_historical_data.insert(m_historical_data.begin(),
          make_entry(m_current_time - seconds(i)));
      }
    }

    void update() {
      m_current_time += seconds(1);
      m_update_signal(make_entry(m_current_time));
    }

    QtPromise<std::vector<Entry>> query_until(
        Beam::Sequence sequence, int max_count) override {
      return QtPromise([=] {
        if(sequence >= Beam::Sequence(to_time_t_milliseconds(m_current_time))) {
          return std::vector<Entry>(m_historical_data.end() - max_count,
            m_historical_data.end());
        } else {
          return std::vector<Entry>(m_historical_data.begin() + m_offset,
            m_historical_data.begin() + max_count + m_offset);
        }
      }, LaunchPolicy::ASYNC);
    }

    connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override {
      return m_update_signal.connect(slot);
    }
  };
}

TEST_SUITE("TimeAndSalesTableModel") {
  TEST_CASE("update") {
    auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
    auto model = TimeAndSalesTableModel(time_and_sales);
    REQUIRE(model.get_row_size() == 0);
    time_and_sales->update();
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(model.get<ptime>(0, 0) == CURRENT_TIME + seconds(1));
    time_and_sales->update();
    REQUIRE(model.get_row_size() == 2);
    REQUIRE(model.get<ptime>(0, 0) == CURRENT_TIME + seconds(2));
    time_and_sales->update();
    REQUIRE(model.get_row_size() == 3);
    REQUIRE(model.get<ptime>(0, 0) == CURRENT_TIME + seconds(3));
  }

  TEST_CASE("update_after_query") {
    run_test([] () {
      auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
      auto model = std::make_shared<TimeAndSalesTableModel>(time_and_sales);
      REQUIRE(model->get_row_size() == 0);
      auto load_count = 5;
      auto [future, promise] = make_future<void>();
      model->connect_end_loading_signal(
        [=, f = std::make_shared<QtFuture<void>>(std::move(future))] {
          REQUIRE(model->get_row_size() == load_count);
          test_model(*model);
          f->resolve();
        });
      model->load_history(load_count);
      wait(std::move(promise));
      REQUIRE(model->get_row_size() == load_count);
      time_and_sales->update();
      REQUIRE(model->get_row_size() == load_count + 1);
      REQUIRE(model->get<ptime>(0, 0) == CURRENT_TIME + seconds(1));
      time_and_sales->update();
      REQUIRE(model->get_row_size() == load_count + 2);
      REQUIRE(model->get<ptime>(0, 0) == CURRENT_TIME + seconds(2));
    });
  }

  TEST_CASE("query_until_now") {
    run_test([] () {
      auto model = std::make_shared<TimeAndSalesTableModel>(
        std::make_shared<TestTimeAndSalesModel>());
      REQUIRE(model->get_row_size() == 0);
      auto load_count = 5;
      auto [future, promise] = make_future<void>();
      model->connect_end_loading_signal(
        [=, f = std::make_shared<QtFuture<void>>(std::move(future))] {
        REQUIRE(model->get_row_size() == load_count);
        test_model(*model);
        f->resolve();
      });
      model->load_history(load_count);
      wait(std::move(promise));
      REQUIRE(model->get_row_size() == load_count);
    });
  }

  TEST_CASE("query_without_duplication") {
    auto model = std::make_shared<TimeAndSalesTableModel>(
      std::make_shared<TestTimeAndSalesModel>());
    run_test_two_loads(*model, 5, 5,
      [&] {
        REQUIRE(model->get_row_size() == 5);
        test_model(*model);
      },
      [&] {
        REQUIRE(model->get_row_size() == 5);
      },
      [&] {
        REQUIRE(model->get_row_size() == 10);
        test_model(*model);
      },
      [&] {
        REQUIRE(model->get_row_size() == 10);
      });
  }

  TEST_CASE("query_with_partial_duplication") {
    auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
    time_and_sales->m_offset = 3;
    auto model =
      std::make_shared<TimeAndSalesTableModel>(std::move(time_and_sales));
    run_test_two_loads(*model, 5, 5,
      [&] {
        REQUIRE(model->get_row_size() == 5);
        test_model(*model);
      },
      [&] {
        REQUIRE(model->get_row_size() == 5);
      },
      [&] {
        REQUIRE(model->get_row_size() == 7);
        test_model(*model);
      },
      [&] {
        REQUIRE(model->get_row_size() == 7);
      });
  }

  TEST_CASE("query_with_all_duplication") {
    auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
    time_and_sales->m_offset = 6;
    auto model =
      std::make_shared<TimeAndSalesTableModel>(std::move(time_and_sales));
    run_test_two_loads(*model, 5, 3,
      [&] {
        REQUIRE(model->get_row_size() == 5);
        test_model(*model);
      },
      [&] {
        REQUIRE(model->get_row_size() == 5);
      },
      [&] {
        REQUIRE(model->get_row_size() == 5);
        test_model(*model);
      },
      [&] {
        REQUIRE(model->get_row_size() == 5);
      });
  }
}
