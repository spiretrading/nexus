#ifndef NEXUS_RISK_STATE_CHECK_HPP
#define NEXUS_RISK_STATE_CHECK_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Performs a check on an account's RiskState.
   * @param <C> The type of AdministrationClient used to monitor an account's
   *        RiskState.
   */
  template<typename C>
  class RiskStateCheck : public OrderSubmissionCheck {
    public:

      /** The type of AdministrationClient used to get the RiskParameters. */
      using AdministrationClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a RiskStateCheck.
       * @param administrationClient Initializes the AdministrationClient.
       */
      template<typename CF>
      RiskStateCheck(CF&& administrationClient);

      void Submit(const OrderInfo& orderInfo) override;

      void Add(const Order& order) override;

    private:
      struct AccountEntry {
        Beam::Threading::Sync<Accounting::PositionOrderBook>
          m_positionOrderBook;
        std::shared_ptr<Beam::StateQueue<RiskService::RiskState>>
          m_riskStateQueue;
        Beam::MultiQueueWriter<ExecutionReport> m_executionReportQueue;

        AccountEntry();
      };
      Beam::GetOptionalLocalPtr<C> m_administrationClient;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<AccountEntry>> m_accountEntries;

      AccountEntry& LoadAccountEntry(
        const Beam::ServiceLocator::DirectoryEntry& account);
  };

  template<typename C>
  RiskStateCheck<C>::AccountEntry::AccountEntry()
    : m_riskStateQueue(
        std::make_shared<Beam::StateQueue<RiskService::RiskState>>()) {}

  template<typename C>
  template<typename CF>
  RiskStateCheck<C>::RiskStateCheck(CF&& administrationClient)
    : m_administrationClient(std::forward<CF>(administrationClient)) {}

  template<typename C>
  void RiskStateCheck<C>::Submit(const OrderInfo& orderInfo) {
    auto& accountEntry = LoadAccountEntry(orderInfo.m_fields.m_account);
    Beam::Threading::With(accountEntry.m_positionOrderBook,
      [&] (auto& positionOrderBook) {
        while(auto report = accountEntry.m_executionReportQueue.TryPop()) {
          positionOrderBook.Update(std::move(*report));
        }
        if(accountEntry.m_riskStateQueue->Peek().m_type !=
            RiskService::RiskState::Type::ACTIVE) {
          if(positionOrderBook.TestOpeningOrderSubmission(orderInfo.m_fields)) {
            BOOST_THROW_EXCEPTION(OrderSubmissionCheckException(
              "Only closing orders are permitted."));
          }
        }
      });
  }

  template<typename C>
  void RiskStateCheck<C>::Add(const Order& order) {
    auto& accountEntry = LoadAccountEntry(order.GetInfo().m_fields.m_account);
    Beam::Threading::With(accountEntry.m_positionOrderBook,
      [&] (auto& positionOrderBook) {
        positionOrderBook.Add(order);
        order.GetPublisher().Monitor(
          accountEntry.m_executionReportQueue.GetWriter());
      });
  }

  template<typename C>
  typename RiskStateCheck<C>::AccountEntry& RiskStateCheck<C>::LoadAccountEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& entry = *m_accountEntries.GetOrInsert(account, [&] {
      auto entry = std::make_shared<AccountEntry>();
      m_administrationClient->GetRiskStatePublisher(account).Monitor(
        entry->m_riskStateQueue);
      return entry;
    });
    entry.m_riskStateQueue->Peek();
    return entry;
  }
}

#endif
