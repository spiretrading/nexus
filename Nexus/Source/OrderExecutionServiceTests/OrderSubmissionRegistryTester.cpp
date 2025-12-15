#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/OrderSubmissionRegistry.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("OrderSubmissionRegistry") {
  TEST_CASE("account_name") {
    auto registry = OrderSubmissionRegistry();
    auto account = DirectoryEntry::make_account(123, "test_account");
    registry.add(account);
    auto called = false;
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence::FIRST;
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence::FIRST;
    auto info = OrderInfo(OrderFields(), DirectoryEntry::make_account(123), 1,
      false, time_from_string("2023-01-01 09:30:00"));
    info.m_fields.m_account = info.m_submission_account;
    registry.publish(info,
      [&] {
        return initial_sequences;
      },
      [&] (const auto& info) {
        called = true;
        REQUIRE(info->get_index() == account);
      });
    REQUIRE(called);
  }
}
