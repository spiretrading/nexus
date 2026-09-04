#include "Spire/BookView/ConsolidatedUserOrderListModel.hpp"
#include <QTimer>

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto TRANSITION_DURATION = 1000;

  bool user_order_comparator(const BookViewModel::UserOrder& left,
      const BookViewModel::UserOrder& right) {
    return std::tie(left.m_price, left.m_destination) <
      std::tie(right.m_price, right.m_destination);
  }

  bool is_displayed(const BookViewModel::UserOrder& order) {
    return order.m_size != 0 && !is_terminal(order.m_status);
  }

  bool is_transitioning(const BookViewModel::UserOrder& order) {
    return is_terminal(order.m_status) ||
      order.m_status == OrderStatus::PARTIALLY_FILLED;
  }
}

ConsolidatedUserOrderListModel::ConsolidatedUserOrderListModel(
    std::shared_ptr<BookViewModel::UserOrderListModel> user_orders)
    : m_user_orders(std::move(user_orders)),
      m_contributions(m_user_orders->get_size()),
      m_transition_count(0) {
  for(auto i = 0; i != m_user_orders->get_size(); ++i) {
    contribute(i);
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

ConsolidatedUserOrderListModel::iterator ConsolidatedUserOrderListModel::find(
    const BookViewModel::UserOrder& order) {
  auto i = std::lower_bound(
    m_model.begin(), m_model.end(), order, user_order_comparator);
  if(i != m_model.end() && (i->m_price != order.m_price ||
      i->m_destination != order.m_destination)) {
    return m_model.end();
  }
  return i;
}

void ConsolidatedUserOrderListModel::contribute(int index) {
  auto& order = m_user_orders->get(index);
  if(!is_displayed(order)) {
    m_contributions[index] = none;
    return;
  }
  m_contributions[index] = order;
  auto i = std::lower_bound(
    m_model.begin(), m_model.end(), order, user_order_comparator);
  if(i == m_model.end() || i->m_price != order.m_price ||
      i->m_destination != order.m_destination) {
    m_model.insert(order, i);
    return;
  }
  auto update = static_cast<BookViewModel::UserOrder>(*i);
  update.m_size += order.m_size;
  update.m_status = order.m_status;
  *i = update;
}

void ConsolidatedUserOrderListModel::withdraw(
    int index, const BookViewModel::UserOrder& order) {
  auto contribution = m_contributions[index];
  if(!contribution) {
    return;
  }
  m_contributions[index] = none;
  auto i = find(*contribution);
  if(i == m_model.end()) {
    return;
  }
  auto update = static_cast<BookViewModel::UserOrder>(*i);
  update.m_size -= contribution->m_size;
  update.m_status = order.m_status;
  if(!is_transitioning(order)) {
    if(update.m_size > 0) {
      *i = update;
    } else {
      m_model.remove(i);
    }
    return;
  }
  if(update.m_size < 0) {
    update.m_size = 0;
  }
  ++m_transition_count;
  update.m_transition = m_transition_count;
  *i = update;
  QTimer::singleShot(TRANSITION_DURATION, this,
    [=, this, transition = m_transition_count, level = *contribution] {
      expire(level, transition);
    });
}

void ConsolidatedUserOrderListModel::expire(
    const BookViewModel::UserOrder& level, int transition) {
  auto i = find(level);
  if(i == m_model.end() || i->m_transition != transition) {
    return;
  }
  if(i->m_size == 0) {
    m_model.remove(i);
    return;
  }
  auto update = static_cast<BookViewModel::UserOrder>(*i);
  update.m_transition = 0;
  *i = update;
}

void ConsolidatedUserOrderListModel::on_operation(const Operation& operation) {
  visit(operation,
    [&] (const AddOperation& operation) {
      m_contributions.insert(
        std::next(m_contributions.begin(), operation.m_index), none);
      contribute(operation.m_index);
    },
    [&] (const MoveOperation& operation) {
      auto contribution = m_contributions[operation.m_source];
      m_contributions.erase(
        std::next(m_contributions.begin(), operation.m_source));
      m_contributions.insert(
        std::next(m_contributions.begin(), operation.m_destination),
        contribution);
    },
    [&] (const PreRemoveOperation& operation) {
      m_removed_order = m_user_orders->get(operation.m_index);
    },
    [&] (const RemoveOperation& operation) {
      withdraw(operation.m_index, m_removed_order);
      m_contributions.erase(
        std::next(m_contributions.begin(), operation.m_index));
    },
    [&] (const UpdateOperation& operation) {
      withdraw(operation.m_index, operation.get_value());
      contribute(operation.m_index);
    });
}
