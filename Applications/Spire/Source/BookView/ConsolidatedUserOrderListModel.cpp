#include "Spire/BookView/ConsolidatedUserOrderListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  bool user_order_comparator(const BookViewModel::UserOrder& left,
      const BookViewModel::UserOrder& right) {
    return std::tie(left.m_price, left.m_destination) <
      std::tie(right.m_price, right.m_destination);
  }
}

ConsolidatedUserOrderListModel::ConsolidatedUserOrderListModel(
    std::shared_ptr<BookViewModel::UserOrderListModel> user_orders)
    : m_user_orders(std::move(user_orders)) {
  for(auto i = 0; i != m_user_orders->get_size(); ++i) {
    on_operation(AddOperation(i));
  }
  m_connection = m_user_orders->connect_operation_signal(
    std::bind_front(&ConsolidatedUserOrderListModel::on_operation, this));
}

int ConsolidatedUserOrderListModel::get_size() const {
  return m_model.get_size();
}

const ConsolidatedUserOrderListModel::Type&
    ConsolidatedUserOrderListModel::get(int index) const {
  return m_model.get(index);
}

connection ConsolidatedUserOrderListModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_model.connect_operation_signal(slot);
}

void ConsolidatedUserOrderListModel::transact(
    const std::function<void ()>& transaction) {
  m_model.transact([&] {
    transaction();
  });
}

void ConsolidatedUserOrderListModel::on_operation(const Operation& operation) {
  visit(operation,
    [&] (const AddOperation& operation) {
      auto& user_order = m_user_orders->get(operation.m_index);
      auto i = std::lower_bound(
        m_model.begin(), m_model.end(), user_order, user_order_comparator);
      if(i == m_model.end() || i->m_price != user_order.m_price ||
          i->m_destination != user_order.m_destination) {
        m_model.insert(user_order, i);
      } else {
        auto update = static_cast<BookViewModel::UserOrder>(*i);
        update.m_size += user_order.m_size;
        *i = update;
      }
    },
    [&] (const UpdateOperation& operation) {
      auto& user_order = operation.get_value();
      auto size_delta = user_order.m_size - operation.get_previous().m_size;
      if(size_delta == 0) {
        return;
      }
      auto i = std::lower_bound(
        m_model.begin(), m_model.end(), user_order, user_order_comparator);
      auto update = static_cast<BookViewModel::UserOrder>(*i);
      update.m_size += user_order.m_size;
      *i = update;
    });
}
