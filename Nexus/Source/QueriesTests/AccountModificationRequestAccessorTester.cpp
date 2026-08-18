#include <doctest/doctest.h>
#include "Nexus/Queries/AccountModificationRequestAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("AccountModificationRequestAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = AccountModificationRequestAccessor::from_parameter(0);
    auto test_member = [] (const MemberAccessExpression& member,
        const std::string& name, const std::type_index& type) {
      REQUIRE(member.get_name() == name);
      REQUIRE(member.get_type() == type);
      REQUIRE(member.get_expression().get_type() ==
        typeid(AccountModificationRequest));
    };
    SUBCASE("id") {
      test_member(accessor.get_id(), "id", typeid(int));
    }
    SUBCASE("type") {
      test_member(accessor.get_type(), "type", typeid(int));
    }
    SUBCASE("account") {
      test_member(accessor.get_account(), "account", typeid(int));
    }
    SUBCASE("submission_account") {
      test_member(accessor.get_submission_account(), "submission_account",
        typeid(int));
    }
    SUBCASE("timestamp") {
      test_member(accessor.get_timestamp(), "timestamp", typeid(ptime));
    }
    SUBCASE("effective_date") {
      test_member(
        accessor.get_effective_date(), "effective_date", typeid(ptime));
    }
  }
}
