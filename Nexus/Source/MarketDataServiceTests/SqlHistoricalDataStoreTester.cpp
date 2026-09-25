#include <algorithm>
#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/HistoricalDataStoreTestSuite.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlHistoricalDataStore =
    SqlHistoricalDataStore<Viper::Sqlite3::Connection>;

  struct Builder {
    auto operator ()() const {
      return TestSqlHistoricalDataStore([] {
        return Viper::Sqlite3::Connection("file::memory:?cache=shared");
      });
    }
  };
}

TEST_SUITE("SqlHistoricalDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(HistoricalDataStoreTestSuite, Builder);

  TEST_CASE("time_and_sale_conditions") {
    auto& columns = get_time_and_sale_row().get_columns();
    auto i = std::ranges::find(columns, "condition_code", &Column::m_name);
    REQUIRE(i != columns.end());
    auto type = dynamic_cast<const VarCharDataType*>(i->m_type.get());
    REQUIRE(type);
    REQUIRE(type->get_max_size() >= 64);
    auto data_store = Builder()();
    auto ticker = parse_ticker("ABX.TSX");
    auto values = std::vector<SequencedTickerTimeAndSale>();
    auto expected = std::vector<SequencedTimeAndSale>();
    for(auto code : {std::string("@"), std::string("AUCTION"),
        std::string("E;B;N"), std::string("D;M;CO;L;P"),
        std::string("U;BA;CA;E;B;N;D;M;CO;L;P"), std::string(64, 'A')}) {
      auto sale = TimeAndSale(
        time_from_string("2026-09-25 14:00:00"),
        Money::ONE, 100, TimeAndSale::Condition({}, code), "TSE", "", "");
      auto sequence = Beam::Sequence(values.size() + 1);
      values.emplace_back(TickerTimeAndSale(sale, ticker), sequence);
      expected.emplace_back(sale, sequence);
    }
    data_store.store(values);
    auto query = TickerQuery();
    query.set_index(ticker);
    query.set_range(Range::TOTAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    REQUIRE(data_store.load_time_and_sales(query) == expected);
  }
}
