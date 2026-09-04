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
      *table.get_source()).get_source();
    auto& entry = source_list->get(source_row);
    return get<BookViewModel::UserOrder>(entry);
  }

  void navigate_between_sides(
      TableCurrentController::CurrentModel& target_model,
      int current_row, int row_size, optional<int> undo_navigation) {
    auto select = [&] (int row) {
      if(row < 0 || row >= row_size) {
        return false;
      }
      auto index = TableIndex(row, 0);
      if(target_model.test(index) == QValidator::State::Invalid) {
        return false;
      }
      target_model.set(index);
      return true;
    };
    if(undo_navigation) {
      select(*undo_navigation);
      return;
    }
    for(auto offset = 0; offset < row_size; ++offset) {
      if(select(current_row + offset) ||
          (offset > 0 && select(current_row - offset))) {
        return;
      }
    }
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

void CurrentUserOrderModel::navigate(
    const SideEntry& from, const SideEntry& to, Side side) {
  auto& current = get();
  if(!current || current->m_side != side || !from.m_current->get()) {
    return;
  }
  auto current_index = *from.m_current->get();
  auto row_size = to.m_table->get_row_size();
  auto undo_navigation = [&] () -> optional<int> {
    if(m_undo_navigation) {
      return m_undo_navigation->get_index();
    }
    return none;
  }();
  navigate_between_sides(
    *to.m_current, current_index.m_row, row_size, undo_navigation);
  m_undo_navigation.emplace(current_index.m_row);
  m_undo_navigation_connection = from.m_table->connect_operation_signal(
    std::bind_front(&CurrentUserOrderModel::on_operation, this));
}

void CurrentUserOrderModel::navigate_to_bids() {
  navigate(m_current_ask, m_current_bid, Side::ASK);
}

void CurrentUserOrderModel::navigate_to_asks() {
  navigate(m_current_bid, m_current_ask, Side::BID);
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

void CurrentUserOrderModel::clear_undo_navigation() {
  m_undo_navigation = none;
  m_undo_navigation_connection.disconnect();
}

void CurrentUserOrderModel::update_side(
    const SideEntry& selected_side, const SideEntry& other_side, Side side,
    const optional<TableIndex>& current_index) {
  if(other_side.m_current->get()) {
    auto blocker = shared_connection_block(other_side.m_connection);
    other_side.m_current->set(none);
  }
  if(!current_index) {
    clear_undo_navigation();
    m_current.set(none);
    return;
  }
  auto& user_order =
    extract_user_order(*selected_side.m_table, *current_index);
  if(!m_current.get() || side != m_current.get()->m_side ||
      user_order != m_current.get()->m_user_order) {
    clear_undo_navigation();
    m_current.set(CurrentUserOrder(user_order, side));
  }
}

void CurrentUserOrderModel::on_bid(const optional<TableIndex>& current_bid) {
  update_side(m_current_bid, m_current_ask, Side::BID, current_bid);
}

void CurrentUserOrderModel::on_ask(const optional<TableIndex>& current_ask) {
  update_side(m_current_ask, m_current_bid, Side::ASK, current_ask);
}

void CurrentUserOrderModel::on_operation(
    const TableModel::Operation& operation) {
  m_undo_navigation->update(operation);
  if(m_undo_navigation->get_index() == -1) {
    clear_undo_navigation();
  }
}
