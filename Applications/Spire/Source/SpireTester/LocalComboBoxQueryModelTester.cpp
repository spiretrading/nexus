#include <doctest/doctest.h>
#include <Beam/Queries/SnapshotLimit.hpp>
#include "Spire/Ui/LocalComboBoxQueryModel.hpp"

using namespace Beam::Queries;
using namespace Spire;

namespace {
const auto VALUES = std::vector<QString>({"A", "AB", "B", "C"});

  auto make_model() {
    auto model = std::make_shared<LocalComboBoxQueryModel>();
    for(auto& value : VALUES) {
      model->add(value);
    }
    return model;
  }
}

TEST_SUITE("ListValueModel") {
  TEST_CASE("empty_query") {
    auto model = make_model();
    auto query_result1 = wait(
      model->query({"", SnapshotLimit::Unlimited()}).then(
        [&] (auto&& result) { return result.Get(); }));
    REQUIRE(std::is_permutation(query_result1.begin(), query_result1.end(),
      VALUES.begin(), VALUES.end(), ComboBox::QueryModel::BinaryPredicate()));
  }

  TEST_CASE("unlimited_query_some") {}

  TEST_CASE("limited_query_some") {}

  TEST_CASE("reentrant_query") {}

  TEST_CASE("reentrant_query_add_data") {}

  TEST_CASE("reentrant_query_remove_data") {}
}
