#include <future>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDriver.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  struct TestOrderSubmissionCheck : OrderSubmissionCheck {
    std::vector<OrderInfo> m_submissions;
    std::vector<std::shared_ptr<const Order>> m_adds;
    std::vector<OrderInfo> m_rejects;

    void submit(const OrderInfo& info) override {
      m_submissions.push_back(info);
    }

    void add(const std::shared_ptr<const Order>& order) override {
      m_adds.push_back(order);
    }

    void reject(const OrderInfo& info) override {
      m_rejects.push_back(info);
    }
  };

  auto make_order_info(const DirectoryEntry& account) {
    auto fields = OrderFields();
    fields.m_account = account;
    fields.m_security = Security("TST", NYSE);
    fields.m_currency = USD;
    fields.m_type = OrderType::LIMIT;
    fields.m_side = Side::BID;
    fields.m_destination = "NYSE";
    fields.m_quantity = 100;
    fields.m_price = Money::ONE;
    return OrderInfo(fields, 1, false, ptime(not_a_date_time));
  }
}

TEST_SUITE("OrderSubmissionCheckDriver") {
  TEST_CASE("successful_submission") {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestOrderExecutionDriver::Operation>>>();
    auto test_driver = TestOrderExecutionDriver(operations);
    auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
    auto check = std::make_unique<TestOrderSubmissionCheck>();
    auto raw_check = check.get();
    checks.push_back(std::move(check));
    auto driver = OrderSubmissionCheckDriver(&test_driver, std::move(checks));
    auto info = make_order_info(DirectoryEntry::MakeAccount(123));
    auto future_order = std::async(std::launch::async, [&] {
      return driver.submit(info);
    });
    auto operation = operations->Pop();
    auto& submit_operation =
      std::get<TestOrderExecutionDriver::SubmitOperation>(*operation);
    REQUIRE(submit_operation.m_info == info);
    auto order = std::make_shared<PrimitiveOrder>(info);
    submit_operation.m_result.SetResult(order);
    auto submitted_order = future_order.get();
    REQUIRE(submitted_order == order);
    REQUIRE(raw_check->m_submissions.size() == 1);
    REQUIRE(raw_check->m_submissions.front() == info);
    REQUIRE(raw_check->m_adds.size() == 1);
    REQUIRE(raw_check->m_adds.front() == order);
    REQUIRE(raw_check->m_rejects.empty());
  }
}
