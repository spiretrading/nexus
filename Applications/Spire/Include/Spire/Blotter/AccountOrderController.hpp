#ifndef SPIRE_ACCOUNT_ORDER_CONTROLLER_HPP
#define SPIRE_ACCOUNT_ORDER_CONTROLLER_HPP
#include <unordered_map>
#include <Beam/Queues/ScopedBaseQueue.hpp>
#include "Spire/Blotter/AccountOrdersModel.hpp"
#include "Spire/Blotter/BlotterController.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

namespace Spire {

  /** Recovers account orders as tasks and applies cancel-on-fill settings. */
  class AccountOrderController {
    public:

      /**
       * Constructs an AccountOrderController.
       * @param controller The account's consolidated blotter controller.
       * @param tasks The blotter's local task source.
       * @param orders The account's order source.
       * @param profile Supplies execution services and cancel-on-fill settings.
       */
      AccountOrderController(
        BlotterController& controller, std::shared_ptr<TasksModel> tasks,
        const std::shared_ptr<AccountOrdersModel>& orders,
        UserProfile& profile);

    private:
      struct Entry {
        std::shared_ptr<Nexus::Order> m_order;
        bool m_is_cancel_submitted = false;
        bool m_is_fill_processed = false;
      };
      BlotterController* m_controller;
      std::shared_ptr<TasksModel> m_tasks;
      UserProfile* m_profile;
      std::vector<Beam::ScopedBaseQueue<>> m_subscriptions;
      std::unordered_map<Nexus::OrderId, Entry> m_orders;
      std::unordered_map<Nexus::OrderId, std::shared_ptr<Task>> m_wrappers;
      QtTaskQueue m_events;
      boost::signals2::scoped_connection m_order_connection;
      boost::signals2::scoped_connection m_live_connection;
      boost::signals2::scoped_connection m_task_connection;

      void recover(const std::shared_ptr<Nexus::Order>& order);
      void monitor(int index);
      void on_order_operation(const OrdersModel::Operation& operation);
      void on_live_order(const std::shared_ptr<Nexus::Order>& order);
      void on_task_operation(const TasksModel::Operation& operation);
      void on_order(const std::shared_ptr<Nexus::Order>& order);
      void on_report(Nexus::OrderId id, const Nexus::ExecutionReport& report);
  };
}

#endif
