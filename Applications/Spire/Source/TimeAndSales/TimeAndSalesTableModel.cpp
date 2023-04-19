#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_list(const TimeAndSale& time_and_sale) {
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    list_model->push(time_and_sale.m_timestamp);
    list_model->push(time_and_sale.m_price);
    list_model->push(time_and_sale.m_size);
    list_model->push(time_and_sale.m_marketCenter);
    list_model->push(time_and_sale.m_condition);
    return list_model;
  }
}

TimeAndSalesTableModel::TimeAndSalesTableModel(
  std::shared_ptr<TimeAndSalesModel> model)
  : m_model(std::move(model)),
    m_source_connection(m_model->connect_update_signal(
      std::bind_front(&TimeAndSalesTableModel::on_update, this))) {
  load_snapshot(Beam::Queries::Sequence::Present(), 20);
  m_entries.connect_operation_signal(std::bind_front(&TimeAndSalesTableModel::on_operation, this));
}

const std::shared_ptr<TimeAndSalesModel>& TimeAndSalesTableModel::get_model() const {
  return m_model;
}

void TimeAndSalesTableModel::set_model(std::shared_ptr<TimeAndSalesModel> model) {
  clear(m_entries);
  m_model = std::move(model);
  m_source_connection = m_model->connect_update_signal(
    std::bind_front(&TimeAndSalesTableModel::on_update, this));
  load_snapshot(Beam::Queries::Sequence::Present(), 20);
}

void TimeAndSalesTableModel::load_history(int max_count) {
  load_snapshot(m_entries.get(m_entries.get_size() - 1).m_time_and_sale.GetSequence(), max_count);
}

BboIndicator TimeAndSalesTableModel::get(int row) const {
  return m_entries.get(row).m_indicator;
}

int TimeAndSalesTableModel::get_row_size() const {
  return m_entries.get_size();
}

int TimeAndSalesTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef TimeAndSalesTableModel::at(int row, int column) const {
  if(column < 0 || column >= get_column_size()) {
    throw std::out_of_range("The column is out of range.");
  }
  return extract_field(m_entries.get(row).m_time_and_sale.GetValue(),
    static_cast<Column>(column));
}

connection TimeAndSalesTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

connection TimeAndSalesTableModel::connect_begin_loading_signal(
    const BeginLoadingSignal::slot_type& slot) const {
  return m_begin_loading_signal.connect(slot);
}

connection TimeAndSalesTableModel::connect_end_loading_signal(
    const EndLoadingSignal::slot_type& slot) const {
  return m_end_loading_signal.connect(slot);
}

AnyRef TimeAndSalesTableModel::extract_field(const TimeAndSale& time_and_sale,
    Column column) const {
  if(column == Column::TIME) {
    return time_and_sale.m_timestamp;
  } else if(column == Column::PRICE) {
    return time_and_sale.m_price;
  } else if(column == Column::SIZE) {
    return time_and_sale.m_size;
  } else if(column == Column::MARKET) {
    return time_and_sale.m_marketCenter;
  } else if(column == Column::CONDITION) {
    return time_and_sale.m_condition;
  }
  return {};
}

void TimeAndSalesTableModel::load_snapshot(Beam::Queries::Sequence last, int count) {
  m_begin_loading_signal();
  m_promise = m_model->query_until(last, count).then(
    [=] (auto&& result) {
      auto& snapshot = result.Get();
      for(auto i = snapshot.rbegin(); i != snapshot.rend(); ++i) {
        m_entries.push(*i);
      }
      m_end_loading_signal();
    });
}

void TimeAndSalesTableModel::on_update(const TimeAndSalesModel::Entry& entry) {
  m_entries.insert(entry, 0);
}

void TimeAndSalesTableModel::on_operation(
    const ListModel<TimeAndSalesModel::Entry>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<TimeAndSalesModel::Entry>::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
        to_list(operation.get_value().m_time_and_sale.GetValue())));
    },
    [&] (const ListModel<TimeAndSalesModel::Entry>::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        to_list(operation.get_value().m_time_and_sale.GetValue())));
    });
}
