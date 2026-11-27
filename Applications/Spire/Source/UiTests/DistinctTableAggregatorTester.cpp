#include <cmath>
#include <limits>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include <QString>
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/DistinctTableAggregator.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("DistinctTableAggregator") {
  TEST_CASE("integer_values") {
    auto aggregate = DistinctTableAggregator<int>();
    auto missing = AnyRef();
    aggregate.add(missing);
    REQUIRE_FALSE(aggregate.get().has_value());
    aggregate.add(5);
    aggregate.add(5);
    aggregate.update(missing, 7);
    aggregate.remove(5);
    REQUIRE(std::any_cast<DistinctCount>(aggregate.get()) == DistinctCount(2));
    aggregate.update(7, missing);
    aggregate.remove(5);
    REQUIRE_FALSE(aggregate.get().has_value());
  }

  TEST_CASE("text_values") {
    auto count = DistinctTableAggregator<QString>();
    auto cad = QString("CAD");
    auto usd = QString("USD");
    count.add(AnyRef());
    REQUIRE_FALSE(count.get().has_value());
    count.add(cad);
    count.add(cad);
    count.add(usd);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 2);
    count.remove(cad);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 2);
    count.update(cad, usd);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 1);
    count.remove(usd);
    count.remove(usd);
    REQUIRE_FALSE(count.get().has_value());
  }

  TEST_CASE("distinct_directory_entries") {
    auto count = DistinctTableAggregator<DirectoryEntry>();
    auto first = DirectoryEntry::make_account(1, "Trading");
    auto second = DirectoryEntry::make_account(2, "Trading");
    auto renamed = DirectoryEntry::make_account(1, "Operations");
    count.add(first);
    count.add(second);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 2);
    auto directory = DirectoryEntry::make_directory(1, "Trading");
    count.add(directory);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 3);
    count.remove(directory);
    count.add(renamed);
    count.remove(first);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 2);
    count.update(second, renamed);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 1);
    count.remove(first);
    count.remove(renamed);
    REQUIRE_FALSE(count.get().has_value());
  }

  TEST_CASE("distinct_floating_point_values") {
    auto count = DistinctTableAggregator<double>();
    auto nan = std::numeric_limits<double>::quiet_NaN();
    count.add(1.0);
    count.add(std::nextafter(1.0, 2.0));
    count.add(nan);
    count.add(-nan);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 3);
    count.remove(nan);
    count.remove(nan);
    REQUIRE(std::any_cast<DistinctCount>(count.get()).m_value == 2);
  }

  TEST_CASE("distinct_side_count") {
    auto aggregate = DistinctTableAggregator<Side>();
    auto missing = AnyRef();
    auto bid = Side(Side::BID);
    auto ask = Side(Side::ASK);
    SUBCASE("empty") {
      aggregate.add(missing);
      REQUIRE_FALSE(aggregate.get().has_value());
      aggregate.update(missing, bid);
      REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 1);
      aggregate.update(bid, missing);
      REQUIRE_FALSE(aggregate.get().has_value());
    }
    SUBCASE("distinct_values") {
      aggregate.add(bid);
      aggregate.add(bid);
      aggregate.add(ask);
      REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 2);
      aggregate.remove(bid);
      REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 2);
      aggregate.update(ask, bid);
      REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 1);
    }
  }

  TEST_CASE("timestamp_distinct_count") {
    auto midnight = time_from_string("2026-11-25 00:00:00");
    auto previous = midnight - seconds(1);
    auto next = time_from_string("2026-11-26 00:00:00");
    auto aggregate = DistinctTableAggregator<ptime>();
    aggregate.add(previous);
    aggregate.add(midnight);
    aggregate.add(previous);
    REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 2);
    aggregate.update(previous, next);
    REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 3);
    aggregate.remove(previous);
    REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 2);
    aggregate.remove(next);
    REQUIRE(std::any_cast<DistinctCount>(aggregate.get()).m_value == 1);
    aggregate.remove(midnight);
    REQUIRE_FALSE(aggregate.get().has_value());
  }
}
