#ifndef NEXUS_RISKSTATECHECK_HPP
#define NEXUS_RISKSTATECHECK_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Sync.hpp>
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class RiskStateCheck
      \brief Performs a check on an account's RiskState.
      \tparam AdministrationClientType The type of AdministrationClient used to
              monitor an account's RiskState.
   */
  template<typename AdministrationClientType>
  class RiskStateCheck : public OrderSubmissionCheck {
    public:

      //! The type of AdministrationClient used to get the RiskParameters.
      using AdministrationClient =
        Beam::GetTryDereferenceType<AdministrationClientType>;

      //! Constructs a RiskStateCheck.
      /*!
        \param administrationClient Initializes the AdministrationClient.
      */
      template<typename AdministrationClientForward>
      RiskStateCheck(AdministrationClientForward&& administrationClient);

      virtual ~RiskStateCheck() = default;

      virtual void Submit(const OrderInfo& orderInfo);

      virtual void Add(const Order& order);

    private:
      struct AccountEntry {
        Beam::Threading::Sync<Accounting::PositionOrderBook>
          m_positionOrderBook;
        std::shared_ptr<Beam::StateQueue<RiskService::RiskState>>
          m_riskStateQueue;
        Beam::MultiQueueWriter<ExecutionReport> m_executionReportQueue;

        AccountEntry();
      };
      Beam::GetOptionalLocalPtr<AdministrationClientType>
        m_administrationClient;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<AccountEntry>> m_accountEntries;

      AccountEntry& LoadAccountEntry(
        const Beam::ServiceLocator::DirectoryEntry& account);
  };

  template<typename AdministrationClientType>
  RiskStateCheck<AdministrationClientType>::AccountEntry::AccountEntry()
      : m_riskStateQueue(std::make_shared<Beam::StateQueue<
          RiskService::RiskState>>()) {}

  template<typename AdministrationClientType>
  template<typename AdministrationClientForward>
  RiskStateCheck<AdministrationClientType>::RiskStateCheck(
      AdministrationClientForward&& administrationClient)
      : m_administrationClient(std::forward<AdministrationClientForward>(
          administrationClient)) {}

  template<typename AdministrationClientType>
  void RiskStateCheck<AdministrationClientType>::Submit(
      const OrderInfo& orderInfo) {
    auto& accountEntry = LoadAccountEntry(orderInfo.m_fields.m_account);
    Beam::Threading::With(accountEntry.m_positionOrderBook,
      [&] (Accounting::PositionOrderBook& positionOrderBook) {
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

  template<typename AdministrationClientType>
  void RiskStateCheck<AdministrationClientType>::Add(const Order& order) {
    auto& accountEntry = LoadAccountEntry(order.GetInfo().m_fields.m_account);
    Beam::Threading::With(accountEntry.m_positionOrderBook,
      [&] (Accounting::PositionOrderBook& positionOrderBook) {
        positionOrderBook.Add(order);
        order.GetPublisher().Monitor(
          accountEntry.m_executionReportQueue.GetWriter());
      });
  }

  template<typename AdministrationClientType>
  typename RiskStateCheck<AdministrationClientType>::AccountEntry&
      RiskStateCheck<AdministrationClientType>::LoadAccountEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& entry = *m_accountEntries.GetOrInsert(account,
      [&] {
        auto entry = std::make_shared<AccountEntry>();
        m_administrationClient->GetRiskStatePublisher(account).Monitor(
          entry->m_riskStateQueue);
        return entry;
      });
    entry.m_riskStateQueue->Peek();
    return entry;
  }
}
}

#endif
