#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/TimeAndSales/LocalTimeAndSalesModel.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  void wait(int milliseconds) {
    auto expire = QTime::currentTime().addMSecs(milliseconds);
    while(QTime::currentTime() < expire) {
      QApplication::processEvents();
    }
  }

  auto make_security() {
    return *ParseWildCardSecurity("MRU.TSX", GetDefaultMarketDatabase(),
      GetDefaultCountryDatabase());
  }
  
  void test_query_result(const std::vector<TimeAndSalesModel::Entry>& result,
      time_duration period, int count, Beam::Queries::Sequence sequence) {
    REQUIRE(result.size() == count);
    if(sequence != Beam::Queries::Sequence::Present()) {
      REQUIRE(result.back().m_time_and_sale.GetSequence() == sequence);
    }
    auto duration = result.back().m_time_and_sale.GetValue().m_timestamp -
      result.front().m_time_and_sale.GetValue().m_timestamp;
    REQUIRE(duration.total_milliseconds() ==
      (period * (count - 1)).total_milliseconds());
  }
}

TEST_SUITE("LocalTimeAndSalesModel") {
  TEST_CASE("update") {
    run_test([] {
      auto model = LocalTimeAndSalesModel(make_security());
      auto period = seconds(1);
      model.set_period(period);
      auto price = Money(100);
      model.set_price(price);
      auto indicator = BboIndicator::AT_BID;
      model.set_bbo_indicator(indicator);
      auto previous_sequence = Beam::Queries::Sequence(0);
      auto previous_time = second_clock::local_time();
      model.connect_update_signal([&] (const auto& entry) {
        auto current_time = second_clock::local_time();
        REQUIRE(current_time - previous_time == period);
        REQUIRE(to_time_t(entry.m_time_and_sale.GetValue().m_timestamp) ==
          to_time_t(current_time));
        REQUIRE(entry.m_time_and_sale.GetSequence() > previous_sequence);
        REQUIRE(entry.m_time_and_sale.GetValue().m_price == price);
        REQUIRE(entry.m_indicator == indicator);
        previous_time = current_time;
        previous_sequence = entry.m_time_and_sale.GetSequence();
      });
      wait(5000);
    });
  }

  TEST_CASE("query") {
    run_test([] {
      auto model = LocalTimeAndSalesModel(make_security());
      auto price = Money(100);
      model.set_price(price);
      auto indicator = BboIndicator::AT_BID;
      model.set_bbo_indicator(indicator);
      auto duration = seconds(0);
      model.set_query_duration(duration);
      auto promise = model.query_until(Beam::Queries::Sequence::Present(), 1);
      auto result = wait(std::move(promise));
      REQUIRE(result.size() == 1);
      REQUIRE(result[0].m_time_and_sale.GetValue().m_price == price);
      REQUIRE(result[0].m_indicator == indicator);
    });
  }

  TEST_CASE("query_duration") {
    run_test([] {
      auto model = LocalTimeAndSalesModel(make_security());
      auto duration = seconds(2);
      model.set_query_duration(duration);
      auto time = microsec_clock::local_time();
      auto promise = model.query_until(Beam::Queries::Sequence::Present(), 1);
      auto result = wait(std::move(promise));
      REQUIRE((microsec_clock::local_time() - time).total_seconds() ==
        duration.total_seconds());
      REQUIRE(result.size() == 1);
    });
  }

  TEST_CASE("query_empty") {
    run_test([] {
      auto model = LocalTimeAndSalesModel(make_security());
      model.set_query_duration(millisec(10));
      auto count = 0;
      auto promise = model.query_until(Beam::Queries::Sequence::Present(),
        count);
      auto result = wait(std::move(promise));
      REQUIRE(result.size() == count);
    });
  }

  TEST_CASE("query_latest") {
    run_test([] {
      auto model = LocalTimeAndSalesModel(make_security());
      auto period = milliseconds(100);
      model.set_period(period);
      model.set_query_duration(millisec(100));
      auto time = second_clock::local_time();
      auto sequence = Beam::Queries::Sequence::Present();
      auto count = 10;
      auto promise = model.query_until(sequence, count);
      auto result = wait(std::move(promise));
      test_query_result(result, period, count, sequence);
      REQUIRE(to_time_t(result.back().m_time_and_sale.GetValue().m_timestamp) ==
        to_time_t(time));
    });
  }

  TEST_CASE("query_past") {
    run_test([] {
      auto model = LocalTimeAndSalesModel(make_security());
      auto period = milliseconds(10);
      model.set_period(period);
      auto time_and_sales = std::vector<TimeAndSalesModel::Entry>();
      model.connect_update_signal([&] (const auto& entry) {
        time_and_sales.emplace_back(entry);
      });
      wait(100);
      auto query_duration = seconds(1);
      model.set_query_duration(query_duration);
      auto sequence = time_and_sales.front().m_time_and_sale.GetSequence();
      auto count = 10;
      auto promise = model.query_until(sequence, count);
      auto result = wait(std::move(promise));
      test_query_result(result, period, count, sequence);
    });
  }

  TEST_CASE("query_middle") {
    run_test([] {
      auto model = LocalTimeAndSalesModel(make_security());
      auto period = milliseconds(10);
      model.set_period(period);
      auto time_and_sales = std::vector<TimeAndSalesModel::Entry>();
      model.connect_update_signal([&] (const auto& entry) {
        time_and_sales.emplace_back(entry);
      });
      wait(100);
      auto query_duration = milliseconds(100);
      model.set_query_duration(query_duration);
      auto sequence =
        (time_and_sales.rbegin() + 2)->m_time_and_sale.GetSequence();
      auto count = 5;
      auto promise = model.query_until(sequence, count);
      auto result = wait(std::move(promise));
      test_query_result(result, period, count, sequence);
    });
  }

  TEST_CASE("query_update_query") {
    run_test([] {
      auto time_and_sales = std::vector<TimeAndSalesModel::Entry>();
      auto model = LocalTimeAndSalesModel(make_security());
      auto period = milliseconds(500);
      model.set_period(period);
      auto query_duration = milliseconds(1000);
      model.set_query_duration(query_duration);
      auto sequence = Beam::Queries::Sequence::Present();
      auto count = 20;
      auto promise = model.query_until(sequence, count);
      time_and_sales = wait(std::move(promise));
      test_query_result(time_and_sales, period, count, sequence);
      model.connect_update_signal([&] (const auto& entry) {
        time_and_sales.emplace_back(entry);
      });
      wait(5000);
      sequence = time_and_sales.front().m_time_and_sale.GetSequence();
      count = 5;
      promise = model.query_until(sequence, count);
      auto result = wait(std::move(promise));
      test_query_result(result, period, count, sequence);
    });
  }
}
