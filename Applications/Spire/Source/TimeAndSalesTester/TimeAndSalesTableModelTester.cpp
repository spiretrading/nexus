#include <deque>
#include <doctest/doctest.h>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/TimeAndSalesTester/TestTimeAndSalesModel.hpp"

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

  Spire::Details::TimeAndSalesEntry make_entry(int index,
      BboIndicator indicator = BboIndicator::UNKNOWN) {
    return Spire::Details::TimeAndSalesEntry(SequencedValue(
      TimeAndSale(CURRENT_TIME + seconds(index), Money::ONE, 100,
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "XNYS"), Beam::Sequence(100 + index)), indicator);
  }

  std::vector<Spire::Details::TimeAndSalesEntry> make_entries(
      int first, int last) {
    auto entries = std::vector<Spire::Details::TimeAndSalesEntry>();
    for(auto i = first; i <= last; ++i) {
      entries.push_back(make_entry(i));
    }
    return entries;
  }

  void require_rows(const TimeAndSalesTableModel& model, int newest) {
    for(auto i = 0; i < model.get_row_size(); ++i) {
      REQUIRE(model.get<ptime>(i, 0) == CURRENT_TIME + seconds(newest - i));
    }
  }
}

TEST_SUITE("TimeAndSalesTableModel") {
  TEST_CASE("publish") {
    auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
    auto model = TimeAndSalesTableModel(time_and_sales);
    REQUIRE(model.get_row_size() == 0);
    auto operations = std::deque<TableModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    time_and_sales->publish(make_entry(1));
    REQUIRE(model.get_row_size() == 1);
    REQUIRE(operations.size() == 1);
    auto add = get<TableModel::AddOperation>(&operations.front());
    REQUIRE(add);
    REQUIRE(add->m_index == 0);
    time_and_sales->publish(make_entry(2));
    REQUIRE(model.get_row_size() == 2);
    require_rows(model, 2);
  }

  TEST_CASE("existing_entries") {
    auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
    time_and_sales->publish(make_entry(1));
    time_and_sales->publish(make_entry(2));
    auto model = TimeAndSalesTableModel(time_and_sales);
    REQUIRE(model.get_row_size() == 2);
    require_rows(model, 2);
  }

  TEST_CASE("load_history") {
    run_test([] {
      auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
      time_and_sales->publish(make_entry(4));
      time_and_sales->publish(make_entry(5));
      auto model = TimeAndSalesTableModel(time_and_sales);
      auto begin_count = 0;
      model.connect_begin_loading_signal([&] {
        ++begin_count;
      });
      auto [future, promise] = make_future<void>();
      auto end = std::make_shared<QtFuture<void>>(std::move(future));
      model.connect_end_loading_signal([=] {
        end->resolve();
      });
      model.load_history(3);
      REQUIRE(begin_count == 1);
      REQUIRE(time_and_sales->get_requests().size() == 1);
      auto request = time_and_sales->pop_request();
      REQUIRE(request.m_max_count == 3);
      request.m_result.resolve(make_entries(1, 3));
      wait(std::move(promise));
      REQUIRE(model.get_row_size() == 5);
      require_rows(model, 5);
    });
  }

  TEST_CASE("load_history_while_loading") {
    run_test([] {
      auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
      auto model = TimeAndSalesTableModel(time_and_sales);
      auto begin_count = 0;
      model.connect_begin_loading_signal([&] {
        ++begin_count;
      });
      auto [future, promise] = make_future<void>();
      auto end = std::make_shared<QtFuture<void>>(std::move(future));
      model.connect_end_loading_signal([=] {
        end->resolve();
      });
      model.load_history(2);
      model.load_history(2);
      REQUIRE(begin_count == 1);
      REQUIRE(time_and_sales->get_requests().size() == 1);
      time_and_sales->pop_request().m_result.resolve(make_entries(1, 2));
      wait(std::move(promise));
      REQUIRE(model.get_row_size() == 2);
      model.load_history(2);
      REQUIRE(begin_count == 2);
      REQUIRE(time_and_sales->get_requests().size() == 1);
    });
  }

  TEST_CASE("set_model") {
    auto first = std::make_shared<TestTimeAndSalesModel>();
    first->publish(make_entry(1));
    first->publish(make_entry(2));
    first->publish(make_entry(3));
    auto model = TimeAndSalesTableModel(first);
    auto removed = 0;
    auto added = 0;
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        if(auto remove = get<TableModel::RemoveOperation>(&operation)) {
          ++removed;
          REQUIRE(remove->m_index == 0);
          REQUIRE(model.get_row_size() == 3 - removed);
        } else if(auto add = get<TableModel::AddOperation>(&operation)) {
          ++added;
          REQUIRE(add->m_index == 0);
          REQUIRE(model.get_row_size() == added);
        }
      }));
    auto second = std::make_shared<TestTimeAndSalesModel>();
    second->publish(make_entry(7));
    second->publish(make_entry(8));
    model.set_model(second);
    REQUIRE(removed == 3);
    REQUIRE(added == 2);
    REQUIRE(model.get_model() == second);
    REQUIRE(model.get_row_size() == 2);
    require_rows(model, 8);
    first->publish(make_entry(4));
    REQUIRE(model.get_row_size() == 2);
    second->publish(make_entry(9));
    REQUIRE(model.get_row_size() == 3);
    require_rows(model, 9);
  }

  TEST_CASE("bbo_indicator") {
    auto time_and_sales = std::make_shared<TestTimeAndSalesModel>();
    time_and_sales->publish(make_entry(1, BboIndicator::AT_BID));
    time_and_sales->publish(make_entry(2, BboIndicator::AT_ASK));
    auto model = TimeAndSalesTableModel(time_and_sales);
    REQUIRE(model.get_bbo_indicator(0) == BboIndicator::AT_ASK);
    REQUIRE(model.get_bbo_indicator(1) == BboIndicator::AT_BID);
    REQUIRE_THROWS_AS(model.get_bbo_indicator(2), std::out_of_range);
  }
}
