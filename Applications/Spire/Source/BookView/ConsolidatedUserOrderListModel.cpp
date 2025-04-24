#include "Spire/BookView/ConsolidatedUserOrderListModel.hpp"
#include <QTimer>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  bool user_order_comparator(const BookViewModel::UserOrder& left,
      const BookViewModel::UserOrder& right) {
    return std::tie(left.m_price, left.m_destination) <
      std::tie(right.m_price, right.m_destination);
  }

  bool is_transient_status(OrderStatus status) {
    return status == OrderStatus::NONE || status == OrderStatus::PENDING_NEW &&
     status == OrderStatus::NEW && status == OrderStatus::PENDING_CANCEL;
  }
}

ConsolidatedUserOrderListModel::ConsolidatedUserOrderListModel(
    std::shared_ptr<BookViewModel::UserOrderListModel> user_orders)
    : m_user_orders(std::move(user_orders)) {
  for(auto i = 0; i != m_user_orders->get_size(); ++i) {
    add(m_user_orders->get(i));
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

void ConsolidatedUserOrderListModel::add(
    const BookViewModel::UserOrder& order) {
  if(order.m_size == 0 || IsTerminal(order.m_status)) {
    return;
  }
  auto i = std::lower_bound(
    m_model.begin(), m_model.end(), order, user_order_comparator);
  if(i == m_model.end() || i->m_price != order.m_price ||
      i->m_destination != order.m_destination) {
    m_model.insert(order, i);
  } else {
    auto update = static_cast<BookViewModel::UserOrder>(*i);
    update.m_size += order.m_size;
    *i = update;
  }
}

void ConsolidatedUserOrderListModel::remove(
    const BookViewModel::UserOrder& order) {
  auto i = std::lower_bound(
    m_model.begin(), m_model.end(), order, user_order_comparator);
  if(i->m_size == order.m_size) {
    if(order.m_size != 0) {
      auto update = static_cast<BookViewModel::UserOrder>(*i);
      update.m_size = 0;
      update.m_status = order.m_status;
      *i = update;
    }
    QTimer::singleShot(1000, this, [=] {
      auto i = std::lower_bound(
        m_model.begin(), m_model.end(), order, user_order_comparator);
      if(i != m_model.end() && i->m_size == 0) {
        m_model.remove(i);
      }
    });
  } else {
    auto update = static_cast<BookViewModel::UserOrder>(*i);
    update.m_size -= order.m_size;
    update.m_status = order.m_status;
    *i = update;
  }
}

void ConsolidatedUserOrderListModel::on_operation(const Operation& operation) {
  visit(operation,
    [&] (const AddOperation& operation) {
      add(m_user_orders->get(operation.m_index));
    },
    [&] (const PreRemoveOperation& operation) {
      m_removed_order = m_user_orders->get(operation.m_index);
    },
    [&] (const RemoveOperation& operation) {
      remove(m_removed_order);
    },
    [&] (const UpdateOperation& operation) {
      auto& user_order = operation.get_value();
      auto& previous_order = operation.get_previous();
      if(user_order.m_destination != previous_order.m_destination ||
          user_order.m_price != previous_order.m_price) {
        remove(previous_order);
        add(user_order);
        return;
      } else if(user_order.m_status != OrderStatus::FILLED &&
          IsTerminal(user_order.m_status)) {
        remove(user_order);
        return;
      }
      auto size_delta = user_order.m_size - operation.get_previous().m_size;
      if(size_delta == 0) {
        return;
      }
      auto i = std::lower_bound(
        m_model.begin(), m_model.end(), user_order, user_order_comparator);
      auto update = static_cast<BookViewModel::UserOrder>(*i);
      update.m_size += size_delta;
      if(!is_transient_status(user_order.m_status)) {
        update.m_status = user_order.m_status;
      }
      *i = update;
      if(update.m_size == 0) {
        remove(update);
      }
    });
}
