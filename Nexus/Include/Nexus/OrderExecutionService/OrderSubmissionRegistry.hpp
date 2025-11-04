#ifndef NEXUS_ORDER_SUBMISSION_REGISTRY_HPP
#define NEXUS_ORDER_SUBMISSION_REGISTRY_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/OrderExecutionService/AccountOrderSubmissionEntry.hpp"

namespace Nexus {

  /** Keeps and updates a registry of Order submissions. */
  class OrderSubmissionRegistry {
    public:

      /** Constructs an OrderSubmissionRegistry. */
      OrderSubmissionRegistry() = default;

      /**
       * Adds an account that is able to submit Orders.
       * @param account The account to add.
       */
      void add(const Beam::DirectoryEntry& account);

      /**
       * Publishes an OrderInfo.
       * @param info The OrderInfo to publish.
       * @param initial_sequence_loader Loads initial Sequences for the account
       *        that submitted the Order.
       * @param f Receives synchronized access to the OrderInfo.
       */
      template<typename InitialSequenceLoader, typename F>
      void publish(const OrderInfo& info,
        const InitialSequenceLoader& initial_sequence_loader, F&& f);

      /**
       * Publishes an ExecutionReport.
       * @param report The ExecutionReport to publish.
       * @param initial_sequence_loader Loads initial Sequences for the account
       *        that submitted the Order.
       * @param f Receives synchronized access to the ExecutionReport.
       */
      template<typename InitialSequenceLoader, typename F>
      void publish(const AccountExecutionReport& report,
        const InitialSequenceLoader& initial_sequence_loader, F&& f);

    private:
      using SyncAccountOrderSubmissionEntry =
        Beam::Sync<AccountOrderSubmissionEntry, Beam::Mutex>;
      Beam::SynchronizedUnorderedSet<Beam::DirectoryEntry> m_accounts;
      Beam::SynchronizedUnorderedMap<Beam::DirectoryEntry,
        std::shared_ptr<SyncAccountOrderSubmissionEntry>, Beam::Mutex>
          m_submission_entries;

      OrderSubmissionRegistry(const OrderSubmissionRegistry&) = delete;
      OrderSubmissionRegistry& operator =(
        const OrderSubmissionRegistry&) = delete;
  };

  inline void OrderSubmissionRegistry::add(
      const Beam::DirectoryEntry& account) {
    m_accounts.update(account);
  }

  template<typename InitialSequenceLoader, typename F>
  void OrderSubmissionRegistry::publish(const OrderInfo& info,
      const InitialSequenceLoader& initial_sequence_loader, F&& f) {
    auto entry = m_submission_entries.get_or_insert(info.m_fields.m_account,
      [&] {
        auto sequences = initial_sequence_loader();
        auto account = m_accounts.get(info.m_fields.m_account);
        return std::make_shared<SyncAccountOrderSubmissionEntry>(
          std::move(account), sequences);
      });
    Beam::with(*entry, [&] (auto& entry) {
      auto sequenced_order_info = entry.publish(info);
      std::forward<F>(f)(sequenced_order_info);
    });
  }

  template<typename InitialSequenceLoader, typename F>
  void OrderSubmissionRegistry::publish(const AccountExecutionReport& report,
      const InitialSequenceLoader& initial_sequence_loader, F&& f) {
    auto entry = m_submission_entries.get_or_insert(report.get_index(), [&] {
      auto sequences = initial_sequence_loader();
      auto account = m_accounts.get(report.get_index());
      return std::make_shared<SyncAccountOrderSubmissionEntry>(
        std::move(account), sequences);
    });
    Beam::with(*entry, [&] (auto& entry) {
      auto sequenced_execution_report = entry.publish(report);
      std::forward<F>(f)(sequenced_execution_report);
    });
  }
}

#endif
