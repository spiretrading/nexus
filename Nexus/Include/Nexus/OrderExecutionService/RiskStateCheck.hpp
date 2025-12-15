#ifndef NEXUS_RISK_STATE_CHECK_HPP
#define NEXUS_RISK_STATE_CHECK_HPP
#include <memory>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/AdministrationService/AdministrationClient.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {

  /**
   * Performs a check on an account's RiskState.
   * @param <C> The type of AdministrationClient used to monitor an account's
   *        RiskState.
   */
  template<typename C> requires IsAdministrationClient<Beam::dereference_t<C>>
  class RiskStateCheck : public OrderSubmissionCheck {
    public:

      /** The type of AdministrationClient used to get the RiskParameters. */
      using AdministrationClient = Beam::dereference_t<C>;

      /**
       * Constructs a RiskStateCheck.
       * @param administration_client Initializes the AdministrationClient.
       */
      template<Beam::Initializes<C> CF>
      explicit RiskStateCheck(CF&& administration_client);

      void submit(const OrderInfo& info) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      struct AccountEntry {
        Beam::Sync<PositionOrderBook> m_position_order_book;
        std::shared_ptr<Beam::StateQueue<RiskState>> m_risk_state_queue;
        Beam::MultiQueueWriter<ExecutionReport> m_execution_report_queue;

        AccountEntry();
      };
      Beam::local_ptr_t<C> m_administration_client;
      Beam::SynchronizedUnorderedMap<
        Beam::DirectoryEntry, std::shared_ptr<AccountEntry>> m_account_entries;

      AccountEntry& load(const Beam::DirectoryEntry& account);
  };

  /**
   * Makes a new RiskStateCheck.
   * @param administration_client Initializes the AdministrationClient.
   */
  template<typename C> requires IsAdministrationClient<Beam::dereference_t<C>>
  auto make_risk_state_check(C&& administration_client) {
    return std::make_unique<RiskStateCheck<std::remove_cvref_t<C>>>(
      std::forward<C>(administration_client));
  }

  template<typename C> requires IsAdministrationClient<Beam::dereference_t<C>>
  RiskStateCheck<C>::AccountEntry::AccountEntry()
    : m_risk_state_queue(std::make_shared<Beam::StateQueue<RiskState>>()) {}

  template<typename C> requires IsAdministrationClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  RiskStateCheck<C>::RiskStateCheck(CF&& administration_client)
    : m_administration_client(std::forward<CF>(administration_client)) {}

  template<typename C> requires IsAdministrationClient<Beam::dereference_t<C>>
  void RiskStateCheck<C>::submit(const OrderInfo& info) {
    auto& account_entry = load(info.m_fields.m_account);
    Beam::with(account_entry.m_position_order_book,
      [&] (auto& position_order_book) {
        while(auto report = account_entry.m_execution_report_queue.try_pop()) {
          position_order_book.update(std::move(*report));
        }
        if(account_entry.m_risk_state_queue->peek().m_type !=
            RiskState::Type::ACTIVE) {
          if(position_order_book.test_opening_order_submission(info.m_fields)) {
            boost::throw_with_location(OrderSubmissionCheckException(
              "Only closing orders are permitted."));
          }
        }
      });
  }

  template<typename C> requires IsAdministrationClient<Beam::dereference_t<C>>
  void RiskStateCheck<C>::add(const std::shared_ptr<Order>& order) {
    auto& account_entry = load(order->get_info().m_fields.m_account);
    Beam::with(account_entry.m_position_order_book,
      [&] (auto& position_order_book) {
        position_order_book.add(order);
        order->get_publisher().monitor(
          account_entry.m_execution_report_queue.get_writer());
      });
  }

  template<typename C> requires IsAdministrationClient<Beam::dereference_t<C>>
  typename RiskStateCheck<C>::AccountEntry& RiskStateCheck<C>::load(
      const Beam::DirectoryEntry& account) {
    auto& entry = *m_account_entries.get_or_insert(account, [&] {
      auto entry = std::make_shared<AccountEntry>();
      m_administration_client->get_risk_state_publisher(account).monitor(
        entry->m_risk_state_queue);
      return entry;
    });
    entry.m_risk_state_queue->peek();
    return entry;
  }
}

#endif
