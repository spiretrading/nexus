#include <sstream>
#include <doctest/doctest.h>
#include "Spire/Spire/ListModel.hpp"

using namespace Spire;

TEST_SUITE("ListModel") {
  TEST_CASE("format_operations") {
    auto test = [] (const auto& operation, const auto& expected) {
      REQUIRE(std::format("{}", operation) == expected);
      REQUIRE(std::format("{:>10}", operation) ==
        std::format("{:>10}", expected));
      REQUIRE(std::format("{:.1}", operation) ==
        std::format("{:.1}", expected));
      auto out = std::ostringstream();
      out << operation;
      REQUIRE(out.str() == expected);
    };
    test(AnyListModel::AddOperation(12), "12");
    test(AnyListModel::PreRemoveOperation(23), "23");
    test(AnyListModel::RemoveOperation(34), "34");
    test(AnyListModel::MoveOperation(45, 56), "(45 56)");
    test(AnyListModel::UpdateOperation(67, std::any(1), std::any("two")), "67");
  }

  TEST_CASE("format_operation_variant") {
    auto test = [] (
        const AnyListModel::Operation& operation, const auto& expected) {
      REQUIRE(std::format("{}", operation) == expected);
      REQUIRE(std::format("{:>30}", operation) ==
        std::format("{:>30}", expected));
      REQUIRE(std::format("{:.10}", operation) ==
        std::format("{:.10}", expected));
      auto out = std::ostringstream();
      out << operation;
      REQUIRE(out.str() == expected);
    };
    test(AnyListModel::AddOperation(12), "(AddOperation 12)");
    test(AnyListModel::PreRemoveOperation(23), "(PreRemoveOperation 23)");
    test(AnyListModel::RemoveOperation(34), "(RemoveOperation 34)");
    test(AnyListModel::MoveOperation(45, 56), "(MoveOperation 45 56)");
    test(AnyListModel::UpdateOperation(67, std::any(1), std::any("two")),
      "(UpdateOperation 67)");
    test(AnyListModel::StartTransaction(), "StartTransaction");
    test(AnyListModel::EndTransaction(), "EndTransaction");
  }
}
