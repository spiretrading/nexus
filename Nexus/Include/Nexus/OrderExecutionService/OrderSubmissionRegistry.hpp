#ifndef NEXUS_ORDER_SUBMISSION_REGISTRY_HPP
#define NEXUS_ORDER_SUBMISSION_REGISTRY_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/OrderExecutionService/AccountOrderSubmissionEntry.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Keeps and updates a registry of Order submissions. */
  class OrderSubmissionRegistry {
    public:

      /** Constructs an OrderSubmissionRegistry. */
      OrderSubmissionRegistry() = default;

      /**
       * Adds an account that is able to submit Orders.
       * @param account The account to add.
       */
      void AddAccount(const Beam::ServiceLocator::DirectoryEntry& account);

      /**
       * Publishes an OrderInfo.
       * @param orderInfo The OrderInfo to publish.
       * @param initialSequenceLoader Loads initial Sequences for the account
       *        that submitted the Order.
       * @param f Receives synchronized access to the OrderInfo.
       */
      template<typename InitialSequenceLoader, typename F>
      void Publish(const OrderInfo& orderInfo,
        const InitialSequenceLoader& initialSequenceLoader, F&& f);

      /**
       * Publishes an ExecutionReport.
       * @param executionReport The ExecutionReport to publish.
       * @param initialSequenceLoader Loads initial Sequences for the account
       *        that submitted the Order.
       * @param f Receives synchronized access to the ExecutionReport.
       */
      template<typename InitialSequenceLoader, typename F>
      void Publish(const AccountExecutionReport& executionReport,
        const InitialSequenceLoader& initialSequenceLoader, F&& f);

    private:
      using SyncAccountOrderSubmissionEntry =
        Beam::Threading::Sync<AccountOrderSubmissionEntry,
        Beam::Threading::Mutex>;
      Beam::SynchronizedUnorderedSet<Beam::ServiceLocator::DirectoryEntry>
        m_accounts;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<SyncAccountOrderSubmissionEntry>,
        Beam::Threading::Mutex> m_submissionEntries;

      OrderSubmissionRegistry(const OrderSubmissionRegistry&) = delete;
      OrderSubmissionRegistry& operator =(
        const OrderSubmissionRegistry&) = delete;
  };

  inline void OrderSubmissionRegistry::AddAccount(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    m_accounts.Update(account);
  }

  template<typename InitialSequenceLoader, typename F>
  void OrderSubmissionRegistry::Publish(const OrderInfo& orderInfo,
      const InitialSequenceLoader& initialSequenceLoader, F&& f) {
    auto entry = m_submissionEntries.GetOrInsert(orderInfo.m_fields.m_account,
      [&] {
        auto initialSequences = initialSequenceLoader();
        auto account = m_accounts.Get(orderInfo.m_fields.m_account);
        return std::make_shared<SyncAccountOrderSubmissionEntry>(account,
          initialSequences);
      });
    Beam::Threading::With(*entry, [&] (auto& entry) {
      auto sequencedOrderInfo = entry.Publish(orderInfo);
      std::forward<F>(f)(sequencedOrderInfo);
    });
  }

  template<typename InitialSequenceLoader, typename F>
  void OrderSubmissionRegistry::Publish(
      const AccountExecutionReport& executionReport,
      const InitialSequenceLoader& initialSequenceLoader, F&& f) {
    auto entry = m_submissionEntries.GetOrInsert(executionReport.GetIndex(),
      [&] {
        auto initialSequences = initialSequenceLoader();
        auto account = m_accounts.Get(executionReport.GetIndex());
        return std::make_shared<SyncAccountOrderSubmissionEntry>(account,
          initialSequences);
      });
    Beam::Threading::With(*entry, [&] (auto& entry) {
      auto sequencedExecutionReport = entry.Publish(executionReport);
      std::forward<F>(f)(sequencedExecutionReport);
    });
  }
}

#endif
