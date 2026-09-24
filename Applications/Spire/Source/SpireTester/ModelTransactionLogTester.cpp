#include <string>
#include <vector>
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
        if(operation.type() == typeid(Model::StartTransaction)) {
          operations.push_back("start");
        } else if(operation.type() == typeid(Model::EndTransaction)) {
          operations.push_back("end");
        } else {
          operations.push_back("operation");
        }
      });
  }
}

TEST_SUITE("ModelTransactionLog") {
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
