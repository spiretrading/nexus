#include "Spire/Ui/TableModelTransactionLog.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableModelTransactionLog::ScopeExit::ScopeExit(std::function<void()> f)
  : m_f(std::move(f)) {}

TableModelTransactionLog::ScopeExit::~ScopeExit() {
  m_f();
}

TableModelTransactionLog::TableModelTransactionLog()
  : m_level(0) {}

void TableModelTransactionLog::push(TableModel::Operation&& operation) {
  if(m_level > 0) {
    m_transaction.m_operations.push_back(std::move(operation));
  } else {
    m_operation_signal(operation);
  }
}

connection TableModelTransactionLog::connect_operation_signal(
    const TableModel::OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}
