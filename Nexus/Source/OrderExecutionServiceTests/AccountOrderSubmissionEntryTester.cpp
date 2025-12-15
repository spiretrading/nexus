#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/AccountOrderSubmissionEntry.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("account_order_submission_entry") {
  TEST_CASE("construct_with_first_sequences") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence::FIRST;
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence::FIRST;
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    REQUIRE(entry.publish(OrderInfo()).get_sequence().get_ordinal() == 1);
    REQUIRE(entry.publish(ExecutionReport()).get_sequence().get_ordinal() == 1);
  }

  TEST_CASE("construct_with_nonzero_sequences") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence(42);
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence(99);
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    REQUIRE(entry.publish(OrderInfo()).get_sequence().get_ordinal() == 43);
    REQUIRE(
      entry.publish(ExecutionReport()).get_sequence().get_ordinal() == 100);
  }

  TEST_CASE("publish_increments_order_info_sequence") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence(10);
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence::FIRST;
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    auto info_1 = entry.publish(OrderInfo());
    auto info_2 = entry.publish(OrderInfo());
    REQUIRE(info_1.get_sequence().get_ordinal() == 11);
    REQUIRE(info_2.get_sequence().get_ordinal() == 12);
  }

  TEST_CASE("publish_sets_account_correctly") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence::FIRST;
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence::FIRST;
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    auto sequenced_info = entry.publish(OrderInfo());
    REQUIRE(sequenced_info->get_index() == account);
  }

  TEST_CASE("publish_multiple_order_info") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence(5);
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence::FIRST;
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    auto sequence_values = std::vector<int>();
    for(auto i = 0; i < 5; ++i) {
      auto info = entry.publish(OrderInfo());
      sequence_values.push_back(info.get_sequence().get_ordinal());
    }
    for(auto i = 1; i < sequence_values.size(); ++i) {
      REQUIRE(sequence_values[i] > sequence_values[i - 1]);
    }
  }

  TEST_CASE("publish_increments_execution_report_sequence") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence::FIRST;
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence(20);
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    auto report_1 = entry.publish(ExecutionReport());
    auto report_2 = entry.publish(ExecutionReport());
    REQUIRE(report_1.get_sequence().get_ordinal() == 21);
    REQUIRE(report_2.get_sequence().get_ordinal() == 22);
  }

  TEST_CASE("publish_sets_account_correctly_execution_report") {
    auto account = DirectoryEntry::make_account(456, "report_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence::FIRST;
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence::FIRST;
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    auto sequenced_report = entry.publish(ExecutionReport());
    REQUIRE(sequenced_report->get_index() == account);
  }

  TEST_CASE("publish_multiple_execution_reports") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    initial_sequences.m_next_order_info_sequence = Beam::Sequence::FIRST;
    initial_sequences.m_next_execution_report_sequence = Beam::Sequence(7);
    auto entry = AccountOrderSubmissionEntry(account, initial_sequences);
    auto sequence_values = std::vector<int>();
    for(auto i = 0; i < 5; ++i) {
      auto report = entry.publish(ExecutionReport());
      sequence_values.push_back(report.get_sequence().get_ordinal());
    }
    for(auto i = 1; i < sequence_values.size(); ++i) {
      REQUIRE(sequence_values[i] > sequence_values[i - 1]);
    }
  }

  TEST_CASE("load_initial_sequences_empty_data_store") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto data_store = LocalOrderExecutionDataStore();
    auto sequences = load_initial_sequences(data_store, account);
    REQUIRE(sequences.m_next_order_info_sequence == Beam::Sequence::FIRST);
    REQUIRE(
      sequences.m_next_execution_report_sequence == Beam::Sequence::FIRST);
  }

  TEST_CASE("load_initial_sequences_nonempty_data_store") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto data_store = LocalOrderExecutionDataStore();
    auto order_fields = OrderFields();
    auto order_info_1 =
      OrderInfo(order_fields, 1, time_from_string("2023-01-01 09:30:00"));
    auto order_info_2 =
      OrderInfo(order_fields, 2, time_from_string("2023-01-01 09:31:00"));
    auto sequenced_info_1 =
      SequencedValue(IndexedValue(order_info_1, account), Beam::Sequence(3));
    auto sequenced_info_2 =
      SequencedValue(IndexedValue(order_info_2, account), Beam::Sequence(5));
    data_store.store(sequenced_info_1);
    data_store.store(sequenced_info_2);
    auto report_1 = ExecutionReport(1, time_from_string("2023-01-01 09:32:00"));
    report_1.m_sequence = 6;
    auto report_2 = ExecutionReport(2, time_from_string("2023-01-01 09:33:00"));
    report_2.m_sequence = 7;
    auto sequenced_report_1 =
      SequencedValue(IndexedValue(report_1, account), Beam::Sequence(6));
    auto sequenced_report_2 =
      SequencedValue(IndexedValue(report_2, account), Beam::Sequence(7));
    data_store.store(sequenced_report_1);
    data_store.store(sequenced_report_2);
    auto sequences = load_initial_sequences(data_store, account);
    REQUIRE(sequences.m_next_order_info_sequence == Beam::Sequence(6));
    REQUIRE(sequences.m_next_execution_report_sequence == Beam::Sequence(8));
  }

  TEST_CASE("load_initial_sequences_mixed_empty_and_nonempty") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto data_store = LocalOrderExecutionDataStore();
    auto order_fields = OrderFields();
    auto order_info =
      OrderInfo(order_fields, 12, time_from_string("2023-01-01 09:30:00"));
    auto sequenced_info =
      SequencedValue(IndexedValue(order_info, account), Beam::Sequence(12));
    data_store.store(sequenced_info);
    auto sequences = load_initial_sequences(data_store, account);
    REQUIRE(sequences.m_next_order_info_sequence == Beam::Sequence(13));
    REQUIRE(
      sequences.m_next_execution_report_sequence == Beam::Sequence::FIRST);
  }
}
