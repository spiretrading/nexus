#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/FixUtilities/FixOrderLog.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Venues;

namespace {
  struct TestApplication : FIX::Application {
    void onCreate(const FIX::SessionID&) override {}
    void onLogon(const FIX::SessionID&) override {}
    void onLogout(const FIX::SessionID&) override {}
    void toAdmin(FIX::Message&, const FIX::SessionID&) override {}
    void toApp(FIX::Message&, const FIX::SessionID&) override {}
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) override {}
    void fromApp(const FIX::Message&, const FIX::SessionID&) override {}
  };

  struct Fixture {
    TestApplication m_application;
    FIX::MemoryStoreFactory m_store_factory;
    FIX::DataDictionaryProvider m_dictionary_provider;
    FIX::Session m_session;
    FixOrderLog m_log;
    FIX::SenderCompID m_sender_comp_id;
    FIX::TargetCompID m_target_comp_id;

    Fixture()
      : m_session([] { return FIX::UtcTimeStamp::now(); },
          m_application, m_store_factory,
          FIX::SessionID("FIX.4.2", "SENDER", "TARGET"),
          m_dictionary_provider,
          FIX::TimeRange(
            FIX::UtcTimeOnly(0, 0, 0), FIX::UtcTimeOnly(23, 59, 59)),
          30, static_cast<FIX::LogFactory*>(nullptr)),
        m_sender_comp_id("SENDER"),
        m_target_comp_id("TARGET") {}
  };
}

TEST_SUITE("FixOrderLog") {
  TEST_CASE("submit_and_find") {
    auto fixture = Fixture();
    auto is_called = false;
    auto fields = make_limit_order_fields(
      parse_ticker("TST.TSX"), Side::BID, "TSX", 100, Money::ONE);
    auto info =
      OrderInfo(fields, 123, time_from_string("2024-05-21 00:00:10.000"));
    auto order = fixture.m_log.submit(info, fixture.m_sender_comp_id,
      fixture.m_target_comp_id,
      [&] (Out<FIX42::NewOrderSingle> new_order_single) {
        is_called = true;
        auto cl_ord_id = FIX::ClOrdID();
        REQUIRE_NOTHROW(new_order_single->get(cl_ord_id));
        REQUIRE(cl_ord_id.getString() == "123");
        auto symbol = FIX::Symbol();
        REQUIRE_NOTHROW(new_order_single->get(symbol));
        REQUIRE(symbol.getString() == "TST");
        auto side = FIX::Side();
        REQUIRE_NOTHROW(new_order_single->get(side));
        REQUIRE(side == FIX::Side_BUY);
        auto order_qty = FIX::OrderQty();
        REQUIRE_NOTHROW(new_order_single->get(order_qty));
        REQUIRE(order_qty == 100);
        auto price = FIX::Price();
        REQUIRE_NOTHROW(new_order_single->get(price));
        REQUIRE(price == 1);
        auto ord_type = FIX::OrdType();
        REQUIRE_NOTHROW(new_order_single->get(ord_type));
        REQUIRE(ord_type == FIX::OrdType_LIMIT);
        auto transact_time = FIX::TransactTime();
        REQUIRE_NOTHROW(new_order_single->get(transact_time));
        auto expected_time = time_from_string("2024-05-21 00:00:10.000");
        auto utc_transact_time = transact_time.getValue();
        REQUIRE(utc_transact_time.getYear() == expected_time.date().year());
        REQUIRE(utc_transact_time.getMonth() == expected_time.date().month());
        REQUIRE(utc_transact_time.getDay() == expected_time.date().day());
        REQUIRE(
          utc_transact_time.getHour() == expected_time.time_of_day().hours());
        REQUIRE(utc_transact_time.getMinute() ==
          expected_time.time_of_day().minutes());
        REQUIRE(utc_transact_time.getSecond() ==
          expected_time.time_of_day().seconds());
        REQUIRE(abs(utc_transact_time.getMillisecond() - static_cast<int>(
          expected_time.time_of_day().total_milliseconds() % 1000)) <= 1);
        auto tif = FIX::TimeInForce();
        REQUIRE_NOTHROW(new_order_single->get(tif));
        REQUIRE(tif == FIX::TimeInForce_DAY);
      });
    REQUIRE(order);
    REQUIRE(is_called);
    auto found = fixture.m_log.find(info.m_id);
    REQUIRE(found);
    REQUIRE(found->get_info() == info);
  }

  TEST_CASE("update_with_empty_reports") {
    auto fixture = Fixture();
    auto fields = make_limit_order_fields(
      parse_ticker("TST.TSX"), Side::BID, "TSX", 100, Money::ONE);
    auto info =
      OrderInfo(fields, 456, time_from_string("2024-05-21 00:00:10.000"));
    auto order = fixture.m_log.submit(info, fixture.m_sender_comp_id,
      fixture.m_target_comp_id, [&] (Out<FIX42::NewOrderSingle>) {});
    REQUIRE(order);
    auto found = fixture.m_log.find(info.m_id);
    REQUIRE(found);
    found->with([&] (auto status, const auto& reports) {
      const_cast<std::vector<ExecutionReport>&>(reports).clear();
    });
    auto execution_session = OrderExecutionSession();
    auto timestamp = time_from_string("2024-05-21 00:00:11.000");
    auto report = ExecutionReport();
    report.m_id = info.m_id;
    SUBCASE("pending_new") {
      report.m_status = OrderStatus::PENDING_NEW;
      fixture.m_log.update(execution_session, info.m_id, report, timestamp);
      found->with([&] (auto status, const auto& reports) {
        REQUIRE(reports.size() == 1);
        REQUIRE(reports.back().m_status == OrderStatus::PENDING_NEW);
      });
    }
    SUBCASE("non_pending_new") {
      report.m_status = OrderStatus::NEW;
      fixture.m_log.update(execution_session, info.m_id, report, timestamp);
      found->with([&] (auto status, const auto& reports) {
        REQUIRE(reports.empty());
      });
    }
  }
}
