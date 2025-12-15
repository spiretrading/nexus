#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto& get_test_request() {
    static auto request =
      AccountModificationRequest(42, AccountModificationRequest::Type::RISK,
        DirectoryEntry::make_account(100, "account"),
        DirectoryEntry::make_account(200, "submitter"),
        time_from_string("2024-07-04 15:30:00"));
    return request;
  }
}

TEST_SUITE("AccountModificationRequest") {
  TEST_CASE("is_terminal") {
    REQUIRE(is_terminal(AccountModificationRequest::Status::GRANTED));
    REQUIRE(is_terminal(AccountModificationRequest::Status::REJECTED));
    REQUIRE(!is_terminal(AccountModificationRequest::Status::NONE));
    REQUIRE(!is_terminal(AccountModificationRequest::Status::PENDING));
    REQUIRE(!is_terminal(AccountModificationRequest::Status::REVIEWED));
    REQUIRE(!is_terminal(AccountModificationRequest::Status::SCHEDULED));
  }

  TEST_CASE("stream") {
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
      DirectoryEntry::make_account(300, "manager"), 7,
      time_from_string("2024-07-05 10:00:00"));
    auto expected =
      std::string("(GRANTED (ACCOUNT 300 manager) 7 2024-Jul-05 10:00:00)");
    REQUIRE(to_string(update) == expected);
    test_round_trip_shuttle(get_test_request(), [] (const auto& received) {
      auto& expected = get_test_request();
      REQUIRE(received.get_id() == expected.get_id());
      REQUIRE(received.get_type() == expected.get_type());
      REQUIRE(received.get_account() == expected.get_account());
      REQUIRE(received.get_submission_account() ==
        expected.get_submission_account());
      REQUIRE(received.get_timestamp() == expected.get_timestamp());
    });
  }

  TEST_CASE("shuttle_update") {
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
      DirectoryEntry::make_account(300, "manager"), 7,
      time_from_string("2024-07-05 10:00:00"));
    test_round_trip_shuttle(update);
  }
}
