#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

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
  load_snapshot(Beam::Queries::Sequence::Present(), 20);
  m_source_connection = m_model->connect_update_signal(
    std::bind_front(&TimeAndSalesTableModel::on_update, this));
}

void TimeAndSalesTableModel::load_history(int max_count) {
  load_snapshot(m_entries.get(m_entries.get_size() - 1).m_entry.m_time_and_sale.GetSequence(), max_count);
}

BboIndicator TimeAndSalesTableModel::get_bbo_indicator(int row) const {
  return m_entries.get(row).m_entry.m_indicator;
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
  return extract_field(m_entries.get(row), static_cast<Column>(column));
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

AnyRef TimeAndSalesTableModel::extract_field(const TimeAndSaleEntry& entry,
    Column column) const {
  if(column == Column::TIME) {
    return entry.m_entry.m_time_and_sale.GetValue().m_timestamp;
  } else if(column == Column::PRICE) {
    return entry.m_entry.m_time_and_sale.GetValue().m_price;
  } else if(column == Column::SIZE) {
    return entry.m_entry.m_time_and_sale.GetValue().m_size;
  } else if(column == Column::MARKET) {
    return entry.m_market;
  } else if(column == Column::CONDITION) {
    return entry.m_entry.m_time_and_sale.GetValue().m_condition;
  }
  return {};
}

void TimeAndSalesTableModel::load_snapshot(Beam::Queries::Sequence last, int count) {
  m_begin_loading_signal();
  m_promise = m_model->query_until(last, count).then(
    [=] (auto&& result) {
      auto& snapshot = result.Get();
      for(auto i = snapshot.rbegin(); i != snapshot.rend(); ++i) {
        m_entries.push({*i, MarketToken(i->m_time_and_sale.GetValue().m_marketCenter)});
      }
      m_end_loading_signal();
    });
}

void TimeAndSalesTableModel::on_update(const TimeAndSalesModel::Entry& entry) {
  m_entries.insert({entry, MarketToken(entry.m_time_and_sale.GetValue().m_marketCenter)}, 0);
}

void TimeAndSalesTableModel::on_operation(
    const ListModel<TimeAndSaleEntry>::Operation& operation) {
  auto to_list = [] (const TimeAndSaleEntry& entry) {
    auto list_model = std::make_shared<ArrayListModel<std::any>>();
    auto& time_and_sale = entry.m_entry.m_time_and_sale.GetValue();
    list_model->push(time_and_sale.m_timestamp);
    list_model->push(time_and_sale.m_price);
    list_model->push(time_and_sale.m_size);
    list_model->push(entry.m_market);
    list_model->push(time_and_sale.m_condition);
    return list_model;
  };
  visit(operation,
    [&] (const ListModel<TimeAndSaleEntry>::AddOperation& operation) {
      m_transaction.push(TableModel::AddOperation(operation.m_index,
        to_list(operation.get_value())));
    },
    [&] (const ListModel<TimeAndSaleEntry>::RemoveOperation& operation) {
      m_transaction.push(TableModel::RemoveOperation(operation.m_index,
        to_list(operation.get_value())));
    });
}
