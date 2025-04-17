#include "Spire/BookView/CurrentUserOrderModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const BookViewModel::UserOrder& extract_user_order(
      const SortedTableModel& table, const TableIndex& index) {
    auto source_row = table.index_to_source(index.m_row);
    auto source_list = static_cast<const ListToTableModel<BookEntry>&>(
      *table.get_source().get()).get_source();
    auto& entry = source_list->get(source_row);
    return get<BookViewModel::UserOrder>(entry);
  }
}

CurrentUserOrderModel::CurrentUserOrderModel(
    std::shared_ptr<SortedTableModel> bid_table,
    std::shared_ptr<TableCurrentController::CurrentModel> current_bid,
    std::shared_ptr<SortedTableModel> ask_table,
    std::shared_ptr<TableCurrentController::CurrentModel> current_ask)
    : m_current_bid(std::move(bid_table), std::move(current_bid)),
      m_current_ask(std::move(ask_table), std::move(current_ask)) {
  m_current_bid.m_connection = m_current_bid.m_current->connect_update_signal(
    std::bind_front(&CurrentUserOrderModel::on_bid, this));
  m_current_ask.m_connection = m_current_ask.m_current->connect_update_signal(
    std::bind_front(&CurrentUserOrderModel::on_ask, this));
}

void CurrentUserOrderModel::navigate_to_bids() {
  if(!get() || get()->m_side != Side::ASK ||
      !m_current_ask.m_current->get()) {
    return;
  }
  auto i = 0;
  auto current_row = m_current_ask.m_current->get()->m_row;
  auto row_size = m_current_bid.m_table->get_row_size();
  while(current_row + i < row_size || current_row - i >= 0) {
    if(current_row + i < row_size &&
        m_current_bid.m_current->set(TableIndex(current_row + i, 0)) !=
          QValidator::State::Invalid) {
      return;
    } else if(current_row - i < row_size &&
        m_current_bid.m_current->set(TableIndex(current_row - i, 0)) !=
          QValidator::State::Invalid) {
      return;
    }
    ++i;
  }
}

void CurrentUserOrderModel::navigate_to_asks() {
  if(!get() || get()->m_side != Side::BID) {
    return;
  }
  auto i = 0;
  auto current_row = m_current_bid.m_current->get()->m_row;
  auto row_size = m_current_ask.m_table->get_row_size();
  while(current_row + i < row_size || current_row - i >= 0) {
    if(current_row + i < row_size &&
        m_current_ask.m_current->set(TableIndex(current_row + i, 0)) !=
          QValidator::State::Invalid) {
      return;
    } else if(current_row - i < row_size &&
        m_current_ask.m_current->set(TableIndex(current_row - i, 0)) !=
          QValidator::State::Invalid) {
      return;
    }
    ++i;
  }
}

const CurrentUserOrderModel::Type& CurrentUserOrderModel::get() const {
  return m_current.get();
}

QValidator::State CurrentUserOrderModel::test(const Type& value) const {
  return m_current.test(value);
}

QValidator::State CurrentUserOrderModel::set(const Type& value) {
  return m_current.set(value);
}

connection CurrentUserOrderModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_current.connect_update_signal(slot);
}

void CurrentUserOrderModel::on_bid(const optional<TableIndex>& current_bid) {
  if(current_bid) {
    if(m_current_ask.m_current->get()) {
      auto blocker = shared_connection_block(m_current_ask.m_connection);
      m_current_ask.m_current->set(none);
    }
    auto& user_order = extract_user_order(*m_current_bid.m_table, *current_bid);
    m_current.set(CurrentUserOrder(user_order, Side::BID));
  } else if(auto& current = m_current_ask.m_current->get()) {
    auto& user_order = extract_user_order(*m_current_ask.m_table, *current);
    m_current.set(CurrentUserOrder(user_order, Side::ASK));
  }
}

void CurrentUserOrderModel::on_ask(const optional<TableIndex>& current_ask) {
  if(current_ask) {
    if(m_current_bid.m_current->get()) {
      auto blocker = shared_connection_block(m_current_bid.m_connection);
      m_current_bid.m_current->set(none);
    }
    auto& user_order = extract_user_order(*m_current_ask.m_table, *current_ask);
    m_current.set(CurrentUserOrder(user_order, Side::ASK));
  } else if(auto& current = m_current_bid.m_current->get()) {
    auto& user_order = extract_user_order(*m_current_bid.m_table, *current);
    m_current.set(CurrentUserOrder(user_order, Side::BID));
  }
}
