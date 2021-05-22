#include "Spire/Ui/RowViewListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

RowViewListModel::RowViewListModel(std::shared_ptr<TableModel> source, int row)
    : m_source(std::move(source)),
      m_row(row) {
  if(row < 0 || row >= m_source->get_row_size()) {
    m_row = -1;
    m_source = nullptr;
  } else {
    m_source_connection = m_source->connect_operation_signal(
      [=] (const auto& operation) { on_operation(operation); });
  }
}

int RowViewListModel::get_size() const {
  if(m_row < 0) {
    return 0;
  }
  return m_source->get_column_size();
}

const std::any& RowViewListModel::at(int index) const {
  if(m_row < 0) {
    throw std::out_of_range("The row is out of range.");
  }
  return m_source->at(m_row, index);
}

QValidator::State RowViewListModel::set(int index, const std::any& value) {
  if(m_row < 0) {
    return QValidator::State::Invalid;
  }
  return m_source->set(m_row, index, value);
}

connection RowViewListModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void RowViewListModel::on_operation(const TableModel::Operation& operation) {
  m_transaction.transact([&] {
    visit<TableModel>(operation,
      [&] (const TableModel::AddOperation& operation) {
        if(m_row >= operation.m_index) {
          ++m_row;
        }
      },
      [&] (const TableModel::MoveOperation& operation) {
        if(m_row == operation.m_source) {
          m_row = operation.m_destination;
          return;
        }
        if(operation.m_source < operation.m_destination) {
          if(m_row > operation.m_source && m_row <= operation.m_destination) {
            --m_row;
          }
        } else if(m_row >= operation.m_destination &&
            m_row < operation.m_source) {
          ++m_row;
        }
      },
      [&] (const TableModel::RemoveOperation& operation) {
        if(m_row == operation.m_index) {
          m_row = -1;
          m_source_connection.disconnect();
          m_source = nullptr;
        } else if(m_row > operation.m_index) {
          --m_row;
        }
      },
      [&] (const TableModel::UpdateOperation& operation) {
        if(m_row == operation.m_row) {
          m_transaction.push(UpdateOperation{operation.m_column});
        }
      });
    });
}
