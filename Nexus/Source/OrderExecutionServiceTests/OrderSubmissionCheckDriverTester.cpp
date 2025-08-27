#include <future>
#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderSubmissionCheckDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDriver.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefaultCurrencies;
using namespace Nexus::DefaultVenues;
using namespace Nexus::Tests;

namespace {
  struct TestOrderSubmissionCheck : OrderSubmissionCheck {
    struct SubmitOperation {
      OrderInfo m_info;
      Eval<void> m_result;
    };

    struct AddOperation {
      std::shared_ptr<Order> m_order;
      Eval<void> m_result;
    };

    struct RejectOperation {
      OrderInfo m_info;
      Eval<void> m_result;
    };

    using Operation =
      std::variant<SubmitOperation, AddOperation, RejectOperation>;

    ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;

    explicit TestOrderSubmissionCheck(
      ScopedQueueWriter<std::shared_ptr<Operation>> operations)
      : m_operations(std::move(operations)) {}

    void submit(const OrderInfo& info) override {
      auto async = Async<void>();
      auto operation =
        std::make_shared<Operation>(SubmitOperation(info, async.GetEval()));
      m_operations.Push(operation);
      async.Get();
    }

    void add(const std::shared_ptr<Order>& order) override {
      auto async = Async<void>();
      auto operation =
        std::make_shared<Operation>(AddOperation(order, async.GetEval()));
      m_operations.Push(operation);
      async.Get();
    }

    void reject(const OrderInfo& info) override {
      auto async = Async<void>();
      auto operation =
        std::make_shared<Operation>(RejectOperation(info, async.GetEval()));
      m_operations.Push(operation);
      async.Get();
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
    auto driver_operations = std::make_shared<
      Queue<std::shared_ptr<TestOrderExecutionDriver::Operation>>>();
    auto test_driver = TestOrderExecutionDriver(driver_operations);
    auto check_operations = std::make_shared<
      Queue<std::shared_ptr<TestOrderSubmissionCheck::Operation>>>();
    auto checks = std::vector<std::unique_ptr<OrderSubmissionCheck>>();
    checks.push_back(
      std::make_unique<TestOrderSubmissionCheck>(check_operations));
    auto driver = OrderSubmissionCheckDriver(&test_driver, std::move(checks));
    auto info = make_order_info(DirectoryEntry::MakeAccount(123));
    auto future_order = std::async(std::launch::async, [&] {
      return driver.submit(info);
    });
    auto check_operation = check_operations->Pop();
    auto& check_submit_operation =
      std::get<TestOrderSubmissionCheck::SubmitOperation>(*check_operation);
    REQUIRE(check_submit_operation.m_info == info);
    check_submit_operation.m_result.SetResult();
    auto driver_operation = driver_operations->Pop();
    auto& driver_submit_operation =
      std::get<TestOrderExecutionDriver::SubmitOperation>(*driver_operation);
    REQUIRE(driver_submit_operation.m_info == info);
    auto order = std::make_shared<PrimitiveOrder>(info);
    driver_submit_operation.m_result.SetResult(order);
    check_operation = check_operations->Pop();
    auto& check_add_operation =
      std::get<TestOrderSubmissionCheck::AddOperation>(*check_operation);
    REQUIRE(check_add_operation.m_order == order);
    check_add_operation.m_result.SetResult();
    auto submitted_order = future_order.get();
    REQUIRE(submitted_order == order);
  }
}
