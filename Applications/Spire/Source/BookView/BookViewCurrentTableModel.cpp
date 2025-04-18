#include "Spire/BookView/BookViewCurrentTableModel.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

BookViewCurrentTableModel::BookViewCurrentTableModel(
  std::shared_ptr<TableModel> table)
  : m_table(std::move(table)),
    m_current(m_table) {}

const BookViewCurrentTableModel::Type& BookViewCurrentTableModel::get() const {
  return m_current.get();
}

QValidator::State BookViewCurrentTableModel::test(const Type& value) const {
  if(!value) {
    return m_current.test(value);
  } else if(value->m_column < 0 ||
      value->m_column >= m_table->get_column_size() || value->m_row < 0 ||
      value->m_row >= m_table->get_row_size()) {
    return QValidator::State::Invalid;
  }
  auto& mpid = m_table->get<Mpid>(value->m_row, 0);
  if(mpid.m_origin != Mpid::Origin::USER_ORDER) {
    return QValidator::State::Invalid;
  }
  return m_current.test(value);
}

QValidator::State BookViewCurrentTableModel::set(const Type& value) {
  auto state = test(value);
  if(state == QValidator::State::Invalid) {
    return state;
  }
  return m_current.set(value);
}

connection BookViewCurrentTableModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_current.connect_update_signal(slot);
}
