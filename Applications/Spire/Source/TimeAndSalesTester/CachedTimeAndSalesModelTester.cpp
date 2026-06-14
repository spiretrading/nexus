#include <doctest/doctest.h>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/TimeAndSales/CachedTimeAndSalesModel.hpp"
#include "Spire/TimeAndSalesTester/TestTimeAndSalesModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto CURRENT_TIME = time_from_string("2024-06-01 10:15:00");

  std::time_t to_time_t_milliseconds(ptime time) {
    return (time -
      time_from_string("1970-01-01 00:00:00")).total_milliseconds();
  }

  TimeAndSalesModel::Entry make_entry(ptime timestamp) {
    return TimeAndSalesModel::Entry(SequencedValue(
      TimeAndSale(timestamp, Money::ONE, 100, TimeAndSale::Condition(
        TimeAndSale::Condition::Type::REGULAR, "@"), "XNYS"),
      Beam::Sequence(to_time_t_milliseconds(timestamp))),
      BboIndicator::UNKNOWN);
  }

  void resolve(TestTimeAndSalesModel::QueryRequest request,
      std::vector<TimeAndSalesModel::Entry> available) {
    if(request.m_sequence != Beam::Sequence::PRESENT) {
      std::erase_if(available, [&] (const auto& entry) {
        return entry.m_time_and_sale.get_sequence() > request.m_sequence;
      });
    }
    if(static_cast<int>(available.size()) > request.m_max_count) {
      available.erase(available.begin(), available.end() - request.m_max_count);
    }
    request.m_result.resolve(std::move(available));
  }
}

TEST_SUITE("CachedTimeAndSalesModel") {
  TEST_CASE("publish_forwards_to_subscriber") {
    auto source = std::make_shared<TestTimeAndSalesModel>();
    auto cache = CachedTimeAndSalesModel(source, 10);
    auto received = std::vector<TimeAndSalesModel::Entry>();
    auto connection = cache.connect_update_signal(
      [&] (const auto& entry) { received.push_back(entry); });
    auto entry = make_entry(CURRENT_TIME);
    source->publish(entry);
    REQUIRE(received.size() == 1);
    REQUIRE(received.front() == entry);
  }

  TEST_CASE("query_present_twice") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto entries = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < count; ++i) {
        entries.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto first = cache.query_until(Beam::Sequence::PRESENT, count);
      REQUIRE(source->get_query_requests().size() == 1);
      auto request = source->pop_query_request();
      REQUIRE(request.m_sequence == Beam::Sequence::PRESENT);
      REQUIRE(request.m_max_count == count);
      request.m_result.resolve(entries);
      auto first_result = wait(std::move(first));
      REQUIRE(first_result == entries);
      auto second = cache.query_until(Beam::Sequence::PRESENT, count);
      REQUIRE(source->get_query_requests().empty());
      auto second_result = wait(std::move(second));
      REQUIRE(second_result == first_result);
    });
  }

  TEST_CASE("query_present_then_larger") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto available = std::vector<TimeAndSalesModel::Entry>();
      available.push_back(make_entry(CURRENT_TIME - seconds(1)));
      for(auto i = 0; i < count; ++i) {
        available.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto first = cache.query_until(Beam::Sequence::PRESENT, count);
      auto second = cache.query_until(Beam::Sequence::PRESENT, count + 1);
      REQUIRE(source->get_query_requests().size() == 1);
      resolve(source->pop_query_request(), available);
      auto recent = std::vector(available.begin() + 1, available.end());
      REQUIRE(wait(std::move(first)) == recent);
      REQUIRE(source->get_query_requests().size() == 1);
      auto backfill_request = source->pop_query_request();
      REQUIRE(backfill_request.m_max_count == 1);
      resolve(std::move(backfill_request), available);
      REQUIRE(wait(std::move(second)) == available);
    });
  }

  TEST_CASE("query_present_single_entry") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto available = std::vector<TimeAndSalesModel::Entry>();
      available.push_back(make_entry(CURRENT_TIME));
      auto query = cache.query_until(Beam::Sequence::PRESENT, count);
      auto recent = cache.query_until(Beam::Sequence::PRESENT, 1);
      REQUIRE(source->get_query_requests().size() == 1);
      resolve(source->pop_query_request(), available);
      REQUIRE(wait(std::move(recent)) == available);
      REQUIRE(source->get_query_requests().size() == 1);
      resolve(source->pop_query_request(), available);
      auto result = wait(std::move(query));
      REQUIRE(result.size() == 1);
      REQUIRE(result == available);
    });
  }

  TEST_CASE("query_present_larger_than_cache") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto available = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < 15; ++i) {
        available.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto request_count = count + 2;
      auto query = cache.query_until(Beam::Sequence::PRESENT, request_count);
      auto recent = cache.query_until(Beam::Sequence::PRESENT, 1);
      REQUIRE(source->get_query_requests().size() == 1);
      resolve(source->pop_query_request(), available);
      REQUIRE(wait(std::move(recent)).size() == 1);
      REQUIRE(source->get_query_requests().size() == 1);
      resolve(source->pop_query_request(), available);
      auto result = wait(std::move(query));
      auto expected =
        std::vector(available.end() - request_count, available.end());
      REQUIRE(result.size() == request_count);
      REQUIRE(result == expected);
    });
  }

  TEST_CASE("query_present_after_publish") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto entries = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < count; ++i) {
        entries.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto first = cache.query_until(Beam::Sequence::PRESENT, count);
      REQUIRE(source->get_query_requests().size() == 1);
      source->pop_query_request().m_result.resolve(entries);
      auto first_result = wait(std::move(first));
      REQUIRE(first_result == entries);
      auto update_count = 3;
      auto published = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < update_count; ++i) {
        auto entry = make_entry(CURRENT_TIME + seconds(count + i));
        source->publish(entry);
        published.push_back(entry);
      }
      auto second = cache.query_until(Beam::Sequence::PRESENT, count);
      REQUIRE(source->get_query_requests().empty());
      auto second_result = wait(std::move(second));
      auto expected = std::vector<TimeAndSalesModel::Entry>(
        entries.begin() + update_count, entries.end());
      expected.insert(expected.end(), published.begin(), published.end());
      REQUIRE(second_result == expected);
    });
  }

  TEST_CASE("query_present_with_concurrent_update") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto snapshot = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < 3; ++i) {
        snapshot.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto live = make_entry(CURRENT_TIME + seconds(3));
      auto query = cache.query_until(Beam::Sequence::PRESENT, 4);
      REQUIRE(source->get_query_requests().size() == 1);
      source->publish(live);
      resolve(source->pop_query_request(), snapshot);
      auto result = wait(std::move(query));
      auto expected = snapshot;
      expected.push_back(live);
      REQUIRE(result == expected);
    });
  }

  TEST_CASE("query_present_with_overlapping_update") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto snapshot = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < 3; ++i) {
        snapshot.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto query = cache.query_until(Beam::Sequence::PRESENT, count);
      auto recent = cache.query_until(Beam::Sequence::PRESENT, 1);
      REQUIRE(source->get_query_requests().size() == 1);
      source->publish(snapshot.back());
      resolve(source->pop_query_request(), snapshot);
      REQUIRE(wait(std::move(recent)).size() == 1);
      REQUIRE(source->get_query_requests().size() == 1);
      resolve(source->pop_query_request(), snapshot);
      auto result = wait(std::move(query));
      REQUIRE(result == snapshot);
    });
  }

  TEST_CASE("query_history") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto entries = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < count; ++i) {
        entries.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto warm = cache.query_until(Beam::Sequence::PRESENT, count);
      source->pop_query_request().m_result.resolve(entries);
      wait(std::move(warm));
      auto history = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < count; ++i) {
        history.push_back(make_entry(CURRENT_TIME - seconds(count - i)));
      }
      auto end_sequence = history.back().m_time_and_sale.get_sequence();
      auto query = cache.query_until(end_sequence, count);
      REQUIRE(source->get_query_requests().size() == 1);
      auto request = source->pop_query_request();
      REQUIRE(request.m_sequence == end_sequence);
      REQUIRE(request.m_max_count == count);
      request.m_result.resolve(history);
      REQUIRE(wait(std::move(query)) == history);
      auto reload = cache.query_until(end_sequence, count);
      REQUIRE(source->get_query_requests().size() == 1);
      auto reload_request = source->pop_query_request();
      REQUIRE(reload_request.m_sequence == end_sequence);
      REQUIRE(reload_request.m_max_count == count);
      reload_request.m_result.resolve(history);
      REQUIRE(wait(std::move(reload)) == history);
    });
  }

  TEST_CASE("query_present_concurrent") {
    run_test([] {
      auto count = 10;
      auto source = std::make_shared<TestTimeAndSalesModel>();
      auto cache = CachedTimeAndSalesModel(source, count);
      auto entries = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < count; ++i) {
        entries.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto first = cache.query_until(Beam::Sequence::PRESENT, count);
      auto second = cache.query_until(Beam::Sequence::PRESENT, count);
      REQUIRE(source->get_query_requests().size() == 1);
      source->pop_query_request().m_result.resolve(entries);
      REQUIRE(wait(std::move(first)) == entries);
      REQUIRE(wait(std::move(second)) == entries);
      REQUIRE(source->get_query_requests().empty());
    });
  }
}
