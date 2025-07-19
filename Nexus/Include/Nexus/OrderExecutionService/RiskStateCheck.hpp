#ifndef NEXUS_RISK_STATE_CHECK_HPP
#define NEXUS_RISK_STATE_CHECK_HPP
#include <memory>
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
       * @param administration_client Initializes the AdministrationClient.
       */
      template<typename CF>
      RiskStateCheck(CF&& administration_client);

      void submit(const OrderInfo& info) override;

      void add(const Order& order) override;

    private:
      struct AccountEntry {
        Beam::Threading::Sync<Accounting::PositionOrderBook>
          m_position_order_book;
        std::shared_ptr<Beam::StateQueue<RiskService::RiskState>>
          m_risk_state_queue;
        Beam::MultiQueueWriter<ExecutionReport> m_execution_report_queue;

        AccountEntry();
      };
      Beam::GetOptionalLocalPtr<C> m_administration_client;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<AccountEntry>> m_account_entries;

      AccountEntry& load(const Beam::ServiceLocator::DirectoryEntry& account);
  };

  /**
   * Makes a new RiskStateCheck.
   * @param administration_client Initializes the AdministrationClient.
   */
  template<typename AdministrationClient>
  auto make_risk_state_check(AdministrationClient&& administration_client) {
    return std::make_unique<RiskStateCheck<std::decay_t<AdministrationClient>>>(
      std::forward<AdministrationClient>(administration_client));
  }

  template<typename C>
  RiskStateCheck<C>::AccountEntry::AccountEntry()
    : m_risk_state_queue(
        std::make_shared<Beam::StateQueue<RiskService::RiskState>>()) {}

  template<typename C>
  template<typename CF>
  RiskStateCheck<C>::RiskStateCheck(CF&& administration_client)
    : m_administration_client(std::forward<CF>(administration_client)) {}

  template<typename C>
  void RiskStateCheck<C>::submit(const OrderInfo& info) {
    auto& account_entry = load(info.m_fields.m_account);
    Beam::Threading::With(account_entry.m_position_order_book,
      [&] (auto& position_order_book) {
        while(auto report = account_entry.m_execution_report_queue.TryPop()) {
          position_order_book.Update(std::move(*report));
        }
        if(account_entry.m_risk_state_queue->Peek().m_type !=
            RiskService::RiskState::Type::ACTIVE) {
          if(position_order_book.TestOpeningOrderSubmission(info.m_fields)) {
            BOOST_THROW_EXCEPTION(OrderSubmissionCheckException(
              "Only closing orders are permitted."));
          }
        }
      });
  }

  template<typename C>
  void RiskStateCheck<C>::add(const Order& order) {
    auto& account_entry = load(order.get_info().m_fields.m_account);
    Beam::Threading::With(account_entry.m_position_order_book,
      [&] (auto& position_order_book) {
        position_order_book.Add(order);
        order.get_publisher().Monitor(
          account_entry.m_execution_report_queue.GetWriter());
      });
  }

  template<typename C>
  typename RiskStateCheck<C>::AccountEntry& RiskStateCheck<C>::load(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto& entry = *m_account_entries.GetOrInsert(account, [&] {
      auto entry = std::make_shared<AccountEntry>();
      m_administration_client->get_risk_state_publisher(account).Monitor(
        entry->m_risk_state_queue);
      return entry;
    });
    entry.m_risk_state_queue->Peek();
    return entry;
  }
}

#endif
