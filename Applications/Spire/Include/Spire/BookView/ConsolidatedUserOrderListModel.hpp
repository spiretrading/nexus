#ifndef SPIRE_CONSOLIDATED_USER_ORDER_LIST_MODEL_HPP
#define SPIRE_CONSOLIDATED_USER_ORDER_LIST_MODEL_HPP
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
       * Constructs a model to consolidated a list of UserOrders.
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
      std::shared_ptr<BookViewModel::UserOrderListModel> m_user_orders;
      std::vector<boost::optional<BookViewModel::UserOrder>> m_contributions;
      ArrayListModel<BookViewModel::UserOrder> m_model;
      BookViewModel::UserOrder m_removed_order;
      int m_transition_count;
      boost::signals2::scoped_connection m_connection;

      iterator find(const BookViewModel::UserOrder& order);
      void contribute(int index);
      void withdraw(int index, const BookViewModel::UserOrder& order);
      void expire(const BookViewModel::UserOrder& level, int transition);
      void on_operation(const Operation& operation);
  };
}

#endif
