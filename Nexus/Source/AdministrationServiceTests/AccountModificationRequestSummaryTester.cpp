#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/lexical_cast.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AccountModificationRequestSummary.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("AccountModificationRequestSummary") {
  TEST_CASE("stream") {
    auto summary = AccountModificationRequestSummary();
    summary.m_request = AccountModificationRequest(42,
      AccountModificationRequest::Type::RISK,
      DirectoryEntry::make_account(100, "account"),
      DirectoryEntry::make_account(200, "submitter"),
      time_from_string("2024-07-04 15:30:00"),
      time_from_string("2024-08-01 00:00:00"));
    summary.m_status = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
      DirectoryEntry::make_account(300, "manager"), 7,
      time_from_string("2024-07-05 10:00:00"));
    summary.m_comment_count = 3;
    auto expected = std::string(
      "((42 RISK (ACCOUNT 100 account) (ACCOUNT 200 submitter) "
      "2024-Jul-04 15:30:00 2024-Aug-01 00:00:00) "
      "(GRANTED (ACCOUNT 300 manager) 7 2024-Jul-05 10:00:00) 3)");
    REQUIRE(lexical_cast<std::string>(summary) == expected);
  }

  TEST_CASE("shuttle") {
    auto summary = AccountModificationRequestSummary();
    summary.m_request = AccountModificationRequest(42,
      AccountModificationRequest::Type::RISK,
      DirectoryEntry::make_account(100, "account"),
      DirectoryEntry::make_account(200, "submitter"),
      time_from_string("2024-07-04 15:30:00"),
      time_from_string("2024-08-01 00:00:00"));
    summary.m_status = AccountModificationRequest::Update(
      AccountModificationRequest::Status::GRANTED,
      DirectoryEntry::make_account(300, "manager"), 7,
      time_from_string("2024-07-05 10:00:00"));
    summary.m_comment_count = 3;
    summary.m_previous_state = Modification(EntitlementModification(
      {DirectoryEntry::make_directory(23, "TSX")}));
    summary.m_modification = Modification(RiskModification(RiskParameters(
      Currencies::USD, 100 * Money::ONE, RiskState::Type::ACTIVE, Money::ONE,
      seconds(5))));
    test_round_trip_shuttle(summary, [&] (const auto& received) {
      REQUIRE(received.m_request.get_id() == summary.m_request.get_id());
      REQUIRE(received.m_status.m_status == summary.m_status.m_status);
      REQUIRE(received.m_comment_count == summary.m_comment_count);
      REQUIRE(get<EntitlementModification>(
        *received.m_previous_state).get_entitlements().size() == 1);
      REQUIRE(get<RiskModification>(
        *received.m_modification).get_parameters() == get<RiskModification>(
          *summary.m_modification).get_parameters());
    });
  }
}
