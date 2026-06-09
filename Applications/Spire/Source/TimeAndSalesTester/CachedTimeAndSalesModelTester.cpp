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
      auto entries = std::vector<TimeAndSalesModel::Entry>();
      for(auto i = 0; i < count; ++i) {
        entries.push_back(make_entry(CURRENT_TIME + seconds(i)));
      }
      auto older_entry = make_entry(CURRENT_TIME - seconds(1));
      auto first = cache.query_until(Beam::Sequence::PRESENT, count);
      auto second = cache.query_until(Beam::Sequence::PRESENT, count + 1);
      REQUIRE(source->get_query_requests().size() == 1);
      auto first_request = source->pop_query_request();
      REQUIRE(first_request.m_sequence == Beam::Sequence::PRESENT);
      REQUIRE(first_request.m_max_count == count);
      first_request.m_result.resolve(entries);
      auto first_result = wait(std::move(first));
      REQUIRE(first_result == entries);
      REQUIRE(source->get_query_requests().size() == 1);
      auto second_request = source->pop_query_request();
      REQUIRE(second_request.m_sequence ==
        entries.front().m_time_and_sale.get_sequence());
      REQUIRE(second_request.m_max_count == 1);
      auto remaining = std::vector<TimeAndSalesModel::Entry>();
      remaining.push_back(older_entry);
      second_request.m_result.resolve(remaining);
      auto second_result = wait(std::move(second));
      auto expected = remaining;
      expected.insert(expected.end(), entries.begin(), entries.end());
      REQUIRE(second_result == expected);
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
