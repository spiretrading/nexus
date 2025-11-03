#ifndef NEXUS_ACCOUNT_ORDER_SUBMISSION_ENTRY_HPP
#define NEXUS_ACCOUNT_ORDER_SUBMISSION_ENTRY_HPP
#include <atomic>
#include <utility>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"

namespace Nexus {

  /** Keeps and updates a registry of Order submissions. */
  class AccountOrderSubmissionEntry {
    public:

      /** Stores the next Sequence to use. */
      struct InitialSequences {

        /** The next Sequence to use for an OrderInfo. */
        Beam::Sequence m_next_order_info_sequence;

        /** The next Sequence to use for an ExecutionReport. */
        Beam::Sequence m_next_execution_report_sequence;
      };

      /**
       * Constructs an AccountOrderSubmissionEntry.
       * @param account The account.
       * @param initial_sequences The initial Sequences to use.
       */
      AccountOrderSubmissionEntry(Beam::DirectoryEntry account,
        InitialSequences initial_sequences) noexcept;

      /**
       * Publishes an OrderInfo.
       * @param info The OrderInfo to publish.
       */
      SequencedAccountOrderInfo publish(const OrderInfo& info);

      /**
       * Publishes an ExecutionReport.
       * @param report The ExecutionReport to publish.
       */
      SequencedAccountExecutionReport publish(const ExecutionReport& report);

    private:
      Beam::DirectoryEntry m_account;
      std::atomic<Beam::Sequence::Ordinal> m_order_sequence;
      std::atomic<Beam::Sequence::Ordinal> m_execution_report_sequence;

      AccountOrderSubmissionEntry(const AccountOrderSubmissionEntry&) = delete;
      AccountOrderSubmissionEntry& operator =(
        const AccountOrderSubmissionEntry&) = delete;
  };

  /**
   * Returns the InitialSequences for an AccountOrderSubmissionEntry.
   * @param data_store The DataStore to load the InitialSequences from.
   * @param account The account to load the InitialSequences for.
   * @return The set of InitialSequences for the specified <i>account</i>.
   */
  AccountOrderSubmissionEntry::InitialSequences load_initial_sequences(
      IsOrderExecutionDataStore auto& data_store,
      const Beam::DirectoryEntry& account) {
    auto query = Beam::make_latest_query(account);
    auto initial_sequences = AccountOrderSubmissionEntry::InitialSequences();
    {
      auto results = data_store.load_order_records(query);
      if(results.empty()) {
        initial_sequences.m_next_order_info_sequence = Beam::Sequence::FIRST;
      } else {
        initial_sequences.m_next_order_info_sequence =
          Beam::increment(results.back().get_sequence());
      }
    }
    {
      auto results = data_store.load_execution_reports(query);
      if(results.empty()) {
        initial_sequences.m_next_execution_report_sequence =
          Beam::Sequence::FIRST;
      } else {
        initial_sequences.m_next_execution_report_sequence =
          Beam::increment(results.back().get_sequence());
      }
    }
    return initial_sequences;
  }

  inline AccountOrderSubmissionEntry::AccountOrderSubmissionEntry(
    Beam::DirectoryEntry account, InitialSequences initial_sequences) noexcept
    : m_account(std::move(account)),
      m_order_sequence(
        initial_sequences.m_next_order_info_sequence.get_ordinal()),
      m_execution_report_sequence(
        initial_sequences.m_next_execution_report_sequence.get_ordinal()) {}

  inline SequencedAccountOrderInfo
      AccountOrderSubmissionEntry::publish(const OrderInfo& info) {
    auto sequence = ++m_order_sequence;
    auto sequenced_order_info = Beam::SequencedValue(
      Beam::IndexedValue(info, m_account), Beam::Sequence(sequence));
    return sequenced_order_info;
  }

  inline SequencedAccountExecutionReport
      AccountOrderSubmissionEntry::publish(const ExecutionReport& report) {
    auto sequence = ++m_execution_report_sequence;
    auto sequenced_execution_report = Beam::SequencedValue(
      Beam::IndexedValue(report, m_account), Beam::Sequence(sequence));
    return sequenced_execution_report;
  }
}

#endif
