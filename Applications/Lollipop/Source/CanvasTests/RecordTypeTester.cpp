#include <doctest/doctest.h>
#include "Spire/Canvas/Types/RecordType.hpp"

using namespace Spire;

TEST_SUITE("RecordType") {
  TEST_CASE("empty_record") {
    auto& type = RecordType::GetEmptyRecordType();
    REQUIRE(type.GetFields().empty());
    REQUIRE(type.GetName() == "Empty Record");
    auto explicitType = MakeRecordType(std::vector<RecordType::Field>());
    REQUIRE(explicitType->GetFields().empty());
    REQUIRE(explicitType->GetName() == "Empty Record");
    auto namedType = MakeRecordType("ABC", std::vector<RecordType::Field>());
    REQUIRE(namedType->GetFields().empty());
    REQUIRE(namedType->GetName() == "ABC Record");
  }
}
