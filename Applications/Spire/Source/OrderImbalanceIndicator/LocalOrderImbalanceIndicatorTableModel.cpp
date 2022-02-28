#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  template<typename T>
  auto get_cell_update(const T& potential,
      const T& current) -> optional<std::any> {
    if(potential == current) {
      return none;
    }
    return std::make_any<T>(potential);
  }
}

void LocalOrderImbalanceIndicatorTableModel::add(
    const OrderImbalance& imbalance) {
  auto& previous_imbalance = m_imbalances[imbalance.m_security];
  if(previous_imbalance.m_imbalance == OrderImbalance()) {
    previous_imbalance = {get_row_size(), imbalance};
    m_table.push(make_row(imbalance));
  } else if(
      previous_imbalance.m_imbalance.m_timestamp < imbalance.m_timestamp) {
    set_row(imbalance, previous_imbalance);
  }
}

void LocalOrderImbalanceIndicatorTableModel::remove(const Security& security) {
  if(auto removed_imbalance = m_imbalances.find(security);
      removed_imbalance != m_imbalances.end()) {
    auto removed_index = removed_imbalance->second.m_row_index;
    for(auto& imbalance : m_imbalances) {
      if(imbalance.second.m_row_index > removed_index) {
        --imbalance.second.m_row_index;
      }
    }
    m_imbalances.erase(removed_imbalance);
    m_table.remove(removed_index);
  }
}

int LocalOrderImbalanceIndicatorTableModel::get_row_size() const {
  return m_table.get_row_size();
}

int LocalOrderImbalanceIndicatorTableModel::get_column_size() const {
  return m_table.get_column_size();
}

const std::any& LocalOrderImbalanceIndicatorTableModel::at(
    int row, int column) const {
  return m_table.at(row, column);
}

connection LocalOrderImbalanceIndicatorTableModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_table.connect_operation_signal(slot);
}

void LocalOrderImbalanceIndicatorTableModel::set_row(
    const OrderImbalance& current, MappedImbalance& previous) {
  m_table.transact([&] {
    auto row = make_row_update(current, previous.m_imbalance);
    for(auto i = std::size_t(1); i < row.size(); ++i) {
      if(auto value = row.at(i)) {
        m_table.set(previous.m_row_index, i, *value);
      }
    }
    previous.m_imbalance = current;
  });
}

std::vector<std::any> LocalOrderImbalanceIndicatorTableModel::make_row(
    const OrderImbalance& imbalance) const {
  return {imbalance.m_security, imbalance.m_side, imbalance.m_size,
    imbalance.m_referencePrice, imbalance.m_size * imbalance.m_referencePrice,
    imbalance.m_timestamp.date(), imbalance.m_timestamp.time_of_day()};
}

std::vector<optional<std::any>>
    LocalOrderImbalanceIndicatorTableModel::make_row_update(
      const OrderImbalance& current, const OrderImbalance& previous) const {
  return {none, get_cell_update(current.m_side, previous.m_side),
    get_cell_update(current.m_size, previous.m_size),
    get_cell_update(current.m_referencePrice, previous.m_referencePrice),
    get_cell_update(current.m_size * current.m_referencePrice,
      previous.m_size * previous.m_referencePrice),
    get_cell_update(current.m_timestamp.date(), previous.m_timestamp.date()),
    get_cell_update(current.m_timestamp.time_of_day(),
      previous.m_timestamp.time_of_day())};
}
