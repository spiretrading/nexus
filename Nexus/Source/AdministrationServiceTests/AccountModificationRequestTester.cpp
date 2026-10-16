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
        time_from_string("2024-07-04 15:30:00"),
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
    REQUIRE(std::format("{}", update) == expected);
    REQUIRE(std::format("{:>60}", update) == std::format("{:>60}", expected));
    REQUIRE(std::format("{:.10}", update) == expected.substr(0, 10));
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

  TEST_CASE("format_request") {
    auto& request = get_test_request();
    auto expected = std::string(
      "(42 RISK (ACCOUNT 100 account) (ACCOUNT 200 submitter) "
      "2024-Jul-04 15:30:00 2024-Jul-04 15:30:00)");
    REQUIRE(std::format("{}", request) == expected);
    REQUIRE(to_string(request) == expected);
    REQUIRE(std::format("{:>110}", request) ==
      std::format("{:>110}", expected));
    REQUIRE(std::format("{:.10}", request) == expected.substr(0, 10));
  }

  TEST_CASE("format_type") {
    using Type = AccountModificationRequest::Type;
    for(auto& [type, expected] : {
        std::pair(Type::ENTITLEMENTS, "ENTITLEMENTS"),
        std::pair(Type::RISK, "RISK"),
        std::pair(Type::COMPLIANCE, "COMPLIANCE"),
        std::pair(static_cast<Type>(-1), "UNKNOWN")}) {
      REQUIRE(std::format("{}", type) == expected);
      REQUIRE(to_string(type) == expected);
    }
    REQUIRE(std::format("{:>6}", Type::RISK) == "  RISK");
    REQUIRE(std::format("{:.2}", Type::RISK) == "RI");
  }

  TEST_CASE("format_status") {
    using Status = AccountModificationRequest::Status;
    for(auto& [status, expected] : {
        std::pair(Status::NONE, "NONE"),
        std::pair(Status::PENDING, "PENDING"),
        std::pair(Status::REVIEWED, "REVIEWED"),
        std::pair(Status::SCHEDULED, "SCHEDULED"),
        std::pair(Status::GRANTED, "GRANTED"),
        std::pair(Status::REJECTED, "REJECTED"),
        std::pair(static_cast<Status>(-1), "UNKNOWN")}) {
      REQUIRE(std::format("{}", status) == expected);
      REQUIRE(to_string(status) == expected);
    }
    REQUIRE(std::format("{:>6}", Status::NONE) == "  NONE");
    REQUIRE(std::format("{:.2}", Status::NONE) == "NO");
  }
}
