#include "Spire/TimeAndSales/TimeAndSalesToTableModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_list(const TimeAndSalesModel::Entry& entry) {
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    list_model->push(entry.m_time_and_sale.GetValue().m_timestamp);
    list_model->push(entry.m_time_and_sale.GetValue().m_price);
    list_model->push(entry.m_time_and_sale.GetValue().m_size);
    list_model->push(entry.m_time_and_sale.GetValue().m_marketCenter);
    list_model->push(entry.m_indicator);
    return list_model;
  }
}

TimeAndSalesToTableModel::TimeAndSalesToTableModel(
  std::shared_ptr<ListModel<Entry>> source)
  : m_source(std::move(source)),
    m_source_connection(m_source->connect_operation_signal(
      std::bind_front(&TimeAndSalesToTableModel::on_operation, this))) {}

int TimeAndSalesToTableModel::get_row_size() const {
  return m_source->get_size();
}

int TimeAndSalesToTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef TimeAndSalesToTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  return extract_field(m_source->get(row), static_cast<Column>(column));
}

connection TimeAndSalesToTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

AnyRef TimeAndSalesToTableModel::extract_field(const Entry& entry,
    Column column) const {
  if(column == Column::TIME) {
    return entry.m_time_and_sale.GetValue().m_timestamp;
  } else if(column == Column::PRICE) {
    return entry.m_time_and_sale.GetValue().m_price;
  } else if(column == Column::SIZE) {
    return entry.m_time_and_sale.GetValue().m_size;
  } else if(column == Column::MARKET) {
    return entry.m_time_and_sale.GetValue().m_marketCenter;
  } else if(column == Column::CONDITION) {
    return entry.m_indicator;
  }
  return {};
}

void TimeAndSalesToTableModel::on_operation(
    const ListModel<Entry>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<Entry>::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const ListModel<Entry>::MoveOperation& operation) {
      m_transaction.push(TableModel::MoveOperation(
        operation.m_source, operation.m_destination));
    },
    [&] (const ListModel<Entry>::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const ListModel<Entry>::UpdateOperation& operation) {
      for(auto i = 0; i < COLUMN_SIZE; ++i) {
        m_transaction.push(TableModel::UpdateOperation(operation.m_index, i,
          to_any(
            extract_field(operation.get_previous(), static_cast<Column>(i))),
          to_any(
            extract_field(operation.get_value(), static_cast<Column>(i)))));
      }
    });
}
