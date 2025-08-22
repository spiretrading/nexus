#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto& get_test_request() {
    static auto request = AccountModificationRequest(
      42, AccountModificationRequest::Type::RISK,
      DirectoryEntry::MakeAccount(100, "account"),
      DirectoryEntry::MakeAccount(200, "submitter"),
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

  TEST_CASE("stream_type") {
    auto ss = std::ostringstream();
    ss << AccountModificationRequest::Type::ENTITLEMENTS;
    REQUIRE(ss.str() == "ENTITLEMENTS");
    ss.str("");
    ss << AccountModificationRequest::Type::RISK;
    REQUIRE(ss.str() == "RISK");
  }

  TEST_CASE("stream_status_enum") {
    auto ss = std::ostringstream();
    ss << AccountModificationRequest::Status::NONE;
    REQUIRE(ss.str() == "NONE");
    ss.str("");
    ss << AccountModificationRequest::Status::PENDING;
    REQUIRE(ss.str() == "PENDING");
    ss.str("");
    ss << AccountModificationRequest::Status::REVIEWED;
    REQUIRE(ss.str() == "REVIEWED");
    ss.str("");
    ss << AccountModificationRequest::Status::SCHEDULED;
    REQUIRE(ss.str() == "SCHEDULED");
    ss.str("");
    ss << AccountModificationRequest::Status::GRANTED;
    REQUIRE(ss.str() == "GRANTED");
    ss.str("");
    ss << AccountModificationRequest::Status::REJECTED;
    REQUIRE(ss.str() == "REJECTED");
  }

  TEST_CASE("stream") {
    auto ss = std::ostringstream();
    auto update = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
      DirectoryEntry::MakeAccount(300, "manager"), 7,
      time_from_string("2024-07-05 10:00:00"));
    ss << update;
    auto expected =
      std::string("(GRANTED (ACCOUNT 300 manager) 7 2024-Jul-05 10:00:00)");
    REQUIRE(ss.str() == expected);
  }

  TEST_CASE("shuttle") {
    Beam::Serialization::Tests::TestRoundTripShuttle(get_test_request(),
      [] (const auto& received) {
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
      DirectoryEntry::MakeAccount(300, "manager"), 7,
      time_from_string("2024-07-05 10:00:00"));
    Beam::Serialization::Tests::TestRoundTripShuttle(update);
  }
}
