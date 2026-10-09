#include <doctest/doctest.h>
#include "Nexus/MarketDataService/DataStoreMarketDataClient.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/TimeAndSales/ServiceTimeAndSalesModel.hpp"

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
      wait(model.load_older(3));
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
      auto model =
        ServiceTimeAndSalesModel(TEST_TICKER, make_client(data_store));
      wait(model.load_older(3));
      REQUIRE(model.get_size() == 3);
      auto indexes = std::vector<int>();
      auto connection = scoped_connection(model.connect_operation_signal(
        [&] (const auto& operation) {
          if(auto add = std::get_if<
              ServiceTimeAndSalesModel::AddOperation>(&operation)) {
            indexes.push_back(add->m_index);
          }
        }));
      wait(model.load_older(5));
      REQUIRE(indexes == std::vector{0, 1});
      REQUIRE(model.get_size() == 5);
      require_entries(model, 1);
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
