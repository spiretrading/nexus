#include <doctest/doctest.h>
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Currencies;
using namespace Nexus::Venues;

namespace {
  struct RecordingComplianceRule : ComplianceRule {
    std::vector<std::shared_ptr<Order>> m_added;

    void add(const std::shared_ptr<Order>& order) override {
      m_added.push_back(order);
    }
  };
}

TEST_SUITE("ComplianceRule") {
  TEST_CASE("restore") {
    auto account = Beam::DirectoryEntry::make_account(123, "test");
    auto fields = make_limit_order_fields(account, parse_ticker("TST.TSX"), CAD,
      Side::BID, "TSX", 100, Money::ONE);
    auto order_a = std::make_shared<PrimitiveOrder>(
      OrderInfo(fields, 1, time_from_string("2024-07-25 10:00:00")));
    auto order_b = std::make_shared<PrimitiveOrder>(
      OrderInfo(fields, 2, time_from_string("2024-07-25 10:00:00")));
    auto rule = RecordingComplianceRule();
    rule.restore(account, InventorySnapshot(),
      std::vector<std::shared_ptr<Order>>{order_a, order_b});
    REQUIRE(rule.m_added.size() == 2);
    REQUIRE(rule.m_added[0] == order_a);
    REQUIRE(rule.m_added[1] == order_b);
  }
}
