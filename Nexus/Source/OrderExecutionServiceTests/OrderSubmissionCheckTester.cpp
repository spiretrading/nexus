#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;

namespace {
  auto make_test_order_fields() {
    auto account = DirectoryEntry::make_account(123, "test");
    auto ticker = parse_ticker("TST.TSX");
    auto currency = CAD;
    auto side = Side::BID;
    auto destination = DefaultDestinations::TSX;
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
}
