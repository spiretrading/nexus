#ifndef SPIRE_CONSOLIDATED_USER_ORDER_LIST_MODEL_HPP
#define SPIRE_CONSOLIDATED_USER_ORDER_LIST_MODEL_HPP
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QObject>
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /**
   * Takes a list of a multitude of UserOrders and consolidates them so that
   * there is only one UserOrder for a given price and destination.
   */
  class ConsolidatedUserOrderListModel :
      public BookViewModel::UserOrderListModel, private QObject {
    public:

      /**
       * Constructs a model that consolidates a list of UserOrders.
       * @param user_orders The list of UserOrders to consolidate.
       */
      explicit ConsolidatedUserOrderListModel(
        std::shared_ptr<BookViewModel::UserOrderListModel> user_orders);

      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      using UserOrder = BookViewModel::UserOrder;
      struct Transition {
        UserOrder m_order;
        std::uint64_t m_transition;
        std::chrono::steady_clock::time_point m_expiry;
      };
      std::shared_ptr<BookViewModel::UserOrderListModel> m_user_orders;
      std::vector<boost::optional<UserOrder>> m_contributions;
      ArrayListModel<UserOrder> m_model;
      UserOrder m_removed_order;
      std::uint64_t m_transition_count;
      std::vector<Transition> m_transitions;
      bool m_is_scheduled;
      boost::signals2::scoped_connection m_connection;

      iterator find(const UserOrder& order);
      void contribute(int index);
      void withdraw(int index, const UserOrder& order);
      void revise(int index, const UserOrder& order);
      void start_transition(UserOrder& order, Nexus::OrderStatus status);
      void expire(const UserOrder& key, std::uint64_t transition);
      void schedule();
      void on_expiry();
      void on_operation(const Operation& operation);
  };
}

#endif
