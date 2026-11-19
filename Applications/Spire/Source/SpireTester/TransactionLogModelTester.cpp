#include <doctest/doctest.h>
#include "Spire/Spire/TransactionLogModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace Spire;

namespace {
  struct Fixture : TransactionLogModel<ListModel<int>> {
    std::vector<int> m_values;

    int get_size() const override {
      return static_cast<int>(m_values.size());
    }

    const int& get(int index) const override {
      return m_values.at(index);
    }

    QValidator::State push(const int& value) override {
      auto index = get_size();
      m_values.push_back(value);
      m_transaction.push(AddOperation(index));
      return QValidator::Acceptable;
    }
  };
}

TEST_SUITE("TransactionLogModel") {
  TEST_CASE("operation_signal") {
    auto fixture = Fixture();
    auto& model = static_cast<ListModel<int>&>(fixture);
    auto operations = std::deque<ListModel<int>::Operation>();
    auto connection = model.connect_operation_signal(
      [&] (const ListModel<int>::Operation& operation) {
        operations.push_back(operation);
      });
    model.start_transaction();
    model.push(3);
    model.push(5);
    model.end_transaction();
    REQUIRE(operations.size() == 4);
    require_list_transaction<int>(operations,
      {ListModel<int>::AddOperation(0), ListModel<int>::AddOperation(1)});
  }
}
