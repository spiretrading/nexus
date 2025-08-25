#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/FixUtilities/FixOrderLog.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

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
}

TEST_SUITE("FixOrderLog") {
  TEST_CASE("submit_and_find") {
    auto settings_stream = std::stringstream();
    settings_stream <<
      "[DEFAULT]\n"
      "ConnectionType=initiator\n"
      "StartTime=00:00:00\n"
      "EndTime=23:59:59\n"
      "heartbeat_interval=30\n"
      "FileStorePath=.\n"
      "SenderCompID=SENDER\n"
      "TargetCompID=TARGET\n"
      "[SESSION]\n"
      "BeginString=FIX.4.2\n"
      "SenderCompID=SENDER\n"
      "TargetCompID=TARGET\n";
    auto settings = FIX::SessionSettings(settings_stream);
    auto application = TestApplication();
    auto store_factory = FIX::MemoryStoreFactory();
    auto dictionary_provider = FIX::DataDictionaryProvider();
    auto start = FIX::UtcTimeOnly(0, 0, 0);
    auto end = FIX::UtcTimeOnly(23, 59, 59);
    auto session_time = FIX::TimeRange(start, end);
    int heartbeat_interval = 30;
    auto log_factory = static_cast<FIX::LogFactory*>(nullptr);
    auto session_id = FIX::SessionID("FIX.4.2", "SENDER", "TARGET");
    auto session = FIX::Session(
      application, store_factory, session_id, dictionary_provider, session_time,
      heartbeat_interval, log_factory);
    auto log = FixOrderLog();
    auto sender_comp_id = FIX::SenderCompID("SENDER");
    auto target_comp_id = FIX::TargetCompID("TARGET");
    bool called = false;
    auto fields = make_limit_order_fields(
      Security("TST", TSX), Side::BID, "TSX", 100, Money::ONE);
    auto info =
      OrderInfo(fields, 123, time_from_string("2024-05-21 00:00:10.000"));
    auto order = log.submit(info, sender_comp_id, target_comp_id,
      [&] (Out<FIX42::NewOrderSingle> new_order_single) {
        called = true;
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
        REQUIRE(utc_transact_time.getHour() ==
          expected_time.time_of_day().hours());
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
    REQUIRE(called);
    auto found = log.find(info.m_id);
    REQUIRE(found);
    REQUIRE(found->get_info() == info);
  }
}
