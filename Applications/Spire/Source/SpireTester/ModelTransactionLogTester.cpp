#include <string>
#include <vector>
#include <boost/signals2/shared_connection_block.hpp>
#include <doctest/doctest.h>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ModelTransactionLog.hpp"

using namespace Spire;

namespace {
  using Model = ListModel<int>;

  auto record(const ModelTransactionLog<Model>& log,
      std::vector<std::string>& operations) {
    return log.connect_operation_signal(
      [&] (const Model::Operation& operation) {
        visit(operation,
          [&] (const Model::StartTransaction&) {
            operations.push_back("start");
          },
          [&] (const Model::EndTransaction&) {
            operations.push_back("end");
          },
          [&] (const auto&) {
            operations.push_back("operation");
          });
      });
  }
}

TEST_SUITE("ModelTransactionLog") {
  TEST_CASE("blocked_late_subscription") {
    auto log = ModelTransactionLog<Model>();
    log.start();
    log.push(Model::AddOperation(0));
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    {
      auto blocker = boost::signals2::shared_connection_block(connection);
      log.end();
    }
    SUBCASE("standalone") {
      log.push(Model::AddOperation(1));
      REQUIRE(operations == std::vector<std::string>{"operation"});
    }
    SUBCASE("next_transaction") {
      log.push(Model::AddOperation(0));
      REQUIRE(operations ==
        std::vector<std::string>{"start", "operation", "end"});
    }
  }

  TEST_CASE("insertion_boundaries") {
    auto log = ModelTransactionLog<Model>(2);
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    log.push(Model::AddOperation(1));
    REQUIRE(operations ==
      std::vector<std::string>{"start", "operation", "end"});
    operations.clear();
    log.push(Model::AddOperation(3));
    REQUIRE(operations == std::vector<std::string>{"operation"});
    log.transact([&] {
      log.push(Model::PreRemoveOperation(1));
      log.push(Model::RemoveOperation(1));
    });
    operations.clear();
    log.push(Model::AddOperation(2));
    log.push(Model::AddOperation(4));
    REQUIRE(operations ==
      std::vector<std::string>{"start", "operation", "end", "operation"});
  }

  TEST_CASE("insertion_from_append") {
    auto log = ModelTransactionLog<Model>();
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    auto has_inserted = false;
    log.connect_operation_signal([&] (const auto& operation) {
      visit(operation, [&] (const Model::AddOperation&) {
        if(!has_inserted) {
          has_inserted = true;
          log.push(Model::AddOperation(0));
        }
      });
    });
    log.push(Model::AddOperation(0));
    REQUIRE(operations ==
      std::vector<std::string>{"operation", "start", "operation", "end"});
  }

  TEST_CASE("pushed_transaction_is_not_duplicated") {
    auto log = ModelTransactionLog<Model>();
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    log.push(Model::StartTransaction());
    log.push(Model::AddOperation(0));
    log.push(Model::AddOperation(1));
    log.push(Model::EndTransaction());
    REQUIRE(operations ==
      std::vector<std::string>{"start", "operation", "operation", "end"});
  }

  TEST_CASE("empty_pushed_transaction_is_suppressed") {
    auto log = ModelTransactionLog<Model>();
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    log.push(Model::StartTransaction());
    log.push(Model::EndTransaction());
    REQUIRE(operations.empty());
  }

  TEST_CASE("nested_pushed_transactions_are_flattened") {
    auto log = ModelTransactionLog<Model>();
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    log.push(Model::StartTransaction());
    log.push(Model::StartTransaction());
    log.push(Model::AddOperation(0));
    log.push(Model::EndTransaction());
    log.push(Model::EndTransaction());
    REQUIRE(operations ==
      std::vector<std::string>{"start", "operation", "end"});
  }

  TEST_CASE("pushed_transaction_joins_an_open_transaction") {
    auto log = ModelTransactionLog<Model>();
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    log.transact([&] {
      log.push(Model::StartTransaction());
      log.push(Model::AddOperation(0));
      log.push(Model::EndTransaction());
    });
    REQUIRE(operations ==
      std::vector<std::string>{"start", "operation", "end"});
  }

  TEST_CASE("move_from_end_transaction") {
    auto log = ModelTransactionLog<Model>(2);
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    auto has_moved = false;
    log.connect_operation_signal([&] (const auto& operation) {
      visit(operation, [&] (const Model::EndTransaction&) {
        if(!has_moved) {
          has_moved = true;
          log.push(Model::MoveOperation(1, 0));
        }
      });
    });
    log.push(Model::MoveOperation(0, 1));
    REQUIRE(operations == std::vector<std::string>{
      "start", "operation", "end", "start", "operation", "end"});
  }

  TEST_CASE("throwing_transaction_does_not_latch_the_log") {
    auto log = ModelTransactionLog<Model>();
    auto operations = std::vector<std::string>();
    auto connection = record(log, operations);
    REQUIRE_THROWS(log.transact([&] {
      throw 42;
    }));
    operations.clear();
    log.push(Model::AddOperation(0));
    REQUIRE(operations == std::vector<std::string>{"operation"});
  }
}
