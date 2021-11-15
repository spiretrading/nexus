#include <doctest/doctest.h>
#include <Beam/Queries/SnapshotLimit.hpp>
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/LocalComboBoxQueryModel.hpp"

using namespace Beam::Queries;
using namespace Spire;
using Query = ComboBox::QueryModel::Query;

namespace {
  const auto VALUES = std::vector<QString>({"A", "AB", "B", "C"});

  auto make_model() {
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& value : VALUES) {
      model->add(value);
    }
    return model;
  }

  auto contains(const QString& value, const std::vector<std::any>& result) {
    return std::find_if(result.begin(), result.end(), [&] (const auto& item) {
      return std::any_cast<QString>(item) == value; }) != result.end();
  }
}

TEST_SUITE("LocalComboBoxQueryModel") {
  TEST_CASE("empty_query") {
    run_test([] {
      auto model = make_model();
      auto promise = model->query(Query::make_empty_query());
      auto result = wait(std::move(promise));
      REQUIRE(result.size() == 4);
      REQUIRE(contains("A", result));
      REQUIRE(contains("AB", result));
      REQUIRE(contains("B", result));
      REQUIRE(contains("C", result));
    });
  }

  TEST_CASE("unlimited_query_with_text") {
    run_test([] {
      auto model = make_model();
      auto promise = model->query({"A", SnapshotLimit::Unlimited()});
      auto result = wait(std::move(promise));
      REQUIRE(result.size() == 2);
      REQUIRE(contains("A", result));
      REQUIRE(contains("AB", result));
    });
  }

  TEST_CASE("limited_query_with_text") {
    run_test([] {
      auto model = make_model();
      auto promise = model->query({"A", SnapshotLimit::FromHead(1)});
      auto result = wait(std::move(promise));
      REQUIRE(result.size() == 1);
      auto result_string = std::any_cast<QString>(result[0]);
      REQUIRE_UNARY(
        result_string == QString("A") || result_string == QString("AB"));
    });
  }
}
