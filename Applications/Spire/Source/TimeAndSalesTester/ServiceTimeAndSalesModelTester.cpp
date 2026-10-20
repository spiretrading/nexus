#include <doctest/doctest.h>
#include "Nexus/MarketDataService/DataStoreMarketDataClient.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/TimeAndSales/ServiceTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto CURRENT_TIME = time_from_string("2024-06-01 10:15:00");
  const auto TEST_TICKER = parse_ticker("TST.TSX");

  void store(LocalHistoricalDataStore& data_store, int index, int second) {
    data_store.store(SequencedTickerTimeAndSale(TickerTimeAndSale(
      TimeAndSale(CURRENT_TIME + seconds(second), Money::ONE, Quantity(index),
        TimeAndSale::Condition(TimeAndSale::Condition::Type::REGULAR, "@"),
        "XNYS"), TEST_TICKER), Beam::Sequence(100 + index)));
  }

  void store_range(LocalHistoricalDataStore& data_store, int first, int last) {
    for(auto i = first; i <= last; ++i) {
      store(data_store, i, i);
    }
  }

  MarketDataClient make_client(LocalHistoricalDataStore& data_store) {
    return MarketDataClient(std::in_place_type<
      DataStoreMarketDataClient<LocalHistoricalDataStore*>>, &data_store);
  }

  void require_entries(const ServiceTimeAndSalesModel& model, int oldest) {
    for(auto i = 0; i < model.get_size(); ++i) {
      REQUIRE(
        model.get(i).m_time_and_sale->m_size == Quantity(oldest + i));
    }
  }
}

TEST_SUITE("ServiceTimeAndSalesModel") {
  TEST_CASE("load_older") {
    run_test([] {
      auto data_store = LocalHistoricalDataStore();
      store_range(data_store, 1, 5);
      auto model =
        ServiceTimeAndSalesModel(TEST_TICKER, make_client(data_store));
      REQUIRE(model.get_size() == 0);
      auto count = 0;
      auto connection = scoped_connection(model.connect_operation_signal(
        [&] (const auto& operation) {
          if(auto add = std::get_if<
              ServiceTimeAndSalesModel::AddOperation>(&operation)) {
            ++count;
            REQUIRE(add->m_index == 0);
            REQUIRE(model.get_size() == count);
            require_entries(model, 6 - count);
            REQUIRE_THROWS_AS(model.get(-1), std::out_of_range);
            REQUIRE_THROWS_AS(model.get(count), std::out_of_range);
          }
        }));
      wait(model.load_older(3));
      REQUIRE(count == 3);
      REQUIRE(model.get_size() == 3);
      require_entries(model, 3);
    });
  }

  TEST_CASE("load_older_without_duplication") {
    run_test([] {
      auto data_store = LocalHistoricalDataStore();
      store_range(data_store, 1, 6);
      auto model =
        ServiceTimeAndSalesModel(TEST_TICKER, make_client(data_store));
      wait(model.load_older(3));
      REQUIRE(model.get_size() == 3);
      require_entries(model, 4);
      wait(model.load_older(3));
      REQUIRE(model.get_size() == 6);
      require_entries(model, 1);
    });
  }

  TEST_CASE("load_older_exceeding_history") {
    run_test([] {
      auto data_store = LocalHistoricalDataStore();
      store_range(data_store, 1, 5);
      auto model = std::make_shared<ServiceTimeAndSalesModel>(
        TEST_TICKER, make_client(data_store));
      auto table = TimeAndSalesTableModel(model);
      wait(model->load_older(3));
      REQUIRE(model->get_size() == 3);
      auto indexes = std::vector<int>();
      auto connection = scoped_connection(model->connect_operation_signal(
        [&] (const auto& operation) {
          if(auto add = std::get_if<
              ServiceTimeAndSalesModel::AddOperation>(&operation)) {
            indexes.push_back(add->m_index);
            REQUIRE(model->get_size() == 3 + std::ssize(indexes));
            require_entries(*model, 3 - static_cast<int>(indexes.size()));
            REQUIRE_THROWS_AS(model->get(model->get_size()), std::out_of_range);
          }
        }));
      auto rows = 3;
      auto table_connection = scoped_connection(table.connect_operation_signal(
        [&] (const auto& operation) {
          if(auto add = std::get_if<TableModel::AddOperation>(&operation)) {
            REQUIRE(add->m_index == rows);
            ++rows;
            REQUIRE(table.get_row_size() == rows);
            for(auto i = 0; i < rows; ++i) {
              REQUIRE(table.get<Quantity>(i, 2) == Quantity(5 - i));
            }
          }
        }));
      wait(model->load_older(5));
      REQUIRE(indexes == std::vector{0, 0});
      REQUIRE(rows == 5);
      REQUIRE(model->get_size() == 5);
      require_entries(*model, 1);
    });
  }

  TEST_CASE("load_older_when_exhausted") {
    run_test([] {
      auto data_store = LocalHistoricalDataStore();
      store_range(data_store, 1, 3);
      auto model =
        ServiceTimeAndSalesModel(TEST_TICKER, make_client(data_store));
      wait(model.load_older(5));
      REQUIRE(model.get_size() == 3);
      auto operation_count = 0;
      auto connection = scoped_connection(model.connect_operation_signal(
        [&] (const auto&) {
          ++operation_count;
        }));
      wait(model.load_older(5));
      REQUIRE(operation_count == 0);
      REQUIRE(model.get_size() == 3);
      require_entries(model, 1);
    });
  }

  TEST_CASE("concurrent_load_older_deduplication") {
    run_test([] {
      auto data_store = LocalHistoricalDataStore();
      store_range(data_store, 1, 5);
      auto model =
        ServiceTimeAndSalesModel(TEST_TICKER, make_client(data_store));
      auto first = model.load_older(3);
      auto second = model.load_older(3);
      wait(std::move(first));
      wait(std::move(second));
      REQUIRE(model.get_size() == 3);
      require_entries(model, 3);
    });
  }

  TEST_CASE("load_older_with_shared_timestamp") {
    run_test([] {
      auto data_store = LocalHistoricalDataStore();
      store(data_store, 1, 1);
      store(data_store, 2, 1);
      store(data_store, 3, 2);
      auto model =
        ServiceTimeAndSalesModel(TEST_TICKER, make_client(data_store));
      wait(model.load_older(2));
      REQUIRE(model.get_size() == 2);
      require_entries(model, 2);
      wait(model.load_older(2));
      REQUIRE(model.get_size() == 3);
      require_entries(model, 1);
    });
  }
}
