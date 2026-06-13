#include <doctest/doctest.h>
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Venues;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = CAD;
    auto side = Side::BID;
    auto destination = Destinations::TSX;
    auto quantity = Quantity(100);
    auto price = Money::ONE;
    return make_limit_order_fields(
      account, ticker, currency, side, destination, quantity, price);
  }

  struct TestOrderSubmissionCheck : OrderSubmissionCheck {
    void submit(const OrderInfo& info) override {}
  };
}

TEST_SUITE("OrderSubmissionCheck") {
  TEST_CASE("add") {
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_test_order_fields();
    auto info = OrderInfo(fields, 123, timestamp);
    auto order = std::make_shared<PrimitiveOrder>(info);
    auto check = TestOrderSubmissionCheck();
    check.add(order);
  }

  TEST_CASE("reject") {
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_test_order_fields();
    auto info = OrderInfo(fields, 123, timestamp);
    auto check = TestOrderSubmissionCheck();
    check.reject(info);
  }

  TEST_CASE("restore") {
    struct RecordingCheck : OrderSubmissionCheck {
      std::vector<std::shared_ptr<Order>> m_added;

      void submit(const OrderInfo& info) override {}

      void add(const std::shared_ptr<Order>& order) override {
        m_added.push_back(order);
      }
    };
    auto timestamp = time_from_string("2024-05-21 00:00:10.000");
    auto fields = make_test_order_fields();
    auto order_a =
      std::make_shared<PrimitiveOrder>(OrderInfo(fields, 1, timestamp));
    auto order_b =
      std::make_shared<PrimitiveOrder>(OrderInfo(fields, 2, timestamp));
    auto check = RecordingCheck();
    check.restore(DirectoryEntry::make_account(123, "test"),
      InventorySnapshot(),
      std::vector<std::shared_ptr<Order>>{order_a, order_b});
    REQUIRE(check.m_added.size() == 2);
    REQUIRE(check.m_added[0] == order_a);
    REQUIRE(check.m_added[1] == order_b);
  }
}
