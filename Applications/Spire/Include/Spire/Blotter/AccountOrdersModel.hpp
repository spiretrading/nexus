#ifndef SPIRE_ACCOUNT_ORDERS_MODEL_HPP
#define SPIRE_ACCOUNT_ORDERS_MODEL_HPP
#include <unordered_set>
#include <Beam/Routines/RoutineHandler.hpp>
#include "Nexus/Clients/Clients.hpp"
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Collects an account's daily and live orders, deduplicated by ID. */
  class AccountOrdersModel :
      public ArrayListModel<std::shared_ptr<Nexus::Order>> {
    public:

      /**
       * Signals an order received from the live stream.
       * @param order The received order.
       */
      using LiveSignal =
        Signal<void (const std::shared_ptr<Nexus::Order>& order)>;

      /**
       * Constructs an AccountOrdersModel.
       * @param account The account whose orders are collected.
       * @param clients Supplies order and time services.
       */
      AccountOrdersModel(
        const Beam::DirectoryEntry& account, Nexus::Clients& clients);

      /** Connects a slot to the live order signal. */
      boost::signals2::connection connect_live_signal(
        const LiveSignal::slot_type& slot) const;

    private:
      mutable LiveSignal m_live_signal;
      std::unordered_set<Nexus::OrderId> m_ids;
      QtTaskQueue m_events;
      Beam::RoutineHandler m_history;

      void on_order(const std::shared_ptr<Nexus::Order>& order);
      void on_live_order(const std::shared_ptr<Nexus::Order>& order);
  };
}

#endif
