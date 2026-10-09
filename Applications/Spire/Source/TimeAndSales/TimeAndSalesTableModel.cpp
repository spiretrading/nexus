#include "Spire/TimeAndSales/TimeAndSalesTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/ReversedListModel.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  AnyRef extract_field(const TimeAndSale& time_and_sale,
      TimeAndSalesTableModel::Column column) {
    if(column == TimeAndSalesTableModel::Column::TIME) {
      return time_and_sale.m_timestamp;
    } else if(column == TimeAndSalesTableModel::Column::PRICE) {
      return time_and_sale.m_price;
    } else if(column == TimeAndSalesTableModel::Column::SIZE) {
      return time_and_sale.m_size;
    } else if(column == TimeAndSalesTableModel::Column::MARKET) {
      return time_and_sale.m_market_center;
    } else if(column == TimeAndSalesTableModel::Column::CONDITION) {
      return time_and_sale.m_condition;
    } else if(column == TimeAndSalesTableModel::Column::BUYER) {
      return time_and_sale.m_buyer_mpid;
    }
    return time_and_sale.m_seller_mpid;
  }

  std::shared_ptr<TableModel> make_table(
      std::shared_ptr<ListModel<TimeAndSalesModel::Type>> source) {
    return std::make_shared<ListToTableModel<TimeAndSalesModel::Type>>(
      std::make_shared<ReversedListModel<TimeAndSalesModel::Type>>(
        std::move(source)),
      TimeAndSalesTableModel::COLUMN_SIZE,
      [] (TimeAndSalesModel::Type& entry, int column) {
        return extract_field(*entry.m_time_and_sale,
          static_cast<TimeAndSalesTableModel::Column>(column));
      });
  }
}

TimeAndSalesTableModel::TimeAndSalesTableModel(
  std::shared_ptr<TimeAndSalesModel> model)
  : m_model(std::move(model)),
    m_source(
      std::make_shared<ProxyListModel<TimeAndSalesModel::Type>>(m_model)),
    m_table(make_table(m_source)),
    m_is_loading(false),
    m_connection(m_table->connect_operation_signal(
      std::bind_front(&TimeAndSalesTableModel::on_operation, this))) {}

const std::shared_ptr<TimeAndSalesModel>&
    TimeAndSalesTableModel::get_model() const {
  return m_model;
}

void TimeAndSalesTableModel::set_model(
    std::shared_ptr<TimeAndSalesModel> model) {
  m_promise.disconnect();
  m_is_loading = false;
  m_model = std::move(model);
  m_source->set_source(m_model);
}

void TimeAndSalesTableModel::load_history(int max_count) {
  if(m_is_loading) {
    return;
  }
  m_is_loading = true;
  m_begin_loading_signal();
  m_promise = m_model->load_older(max_count).then([=] (auto&& result) {
    m_is_loading = false;
    m_end_loading_signal();
  });
}

BboIndicator TimeAndSalesTableModel::get_bbo_indicator(int row) const {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  return m_source->get(m_source->get_size() - 1 - row).m_indicator;
}

int TimeAndSalesTableModel::get_row_size() const {
  return m_table->get_row_size();
}

int TimeAndSalesTableModel::get_column_size() const {
  return COLUMN_SIZE;
}

AnyRef TimeAndSalesTableModel::at(int row, int column) const {
  if(row < 0 || row >= get_row_size() || column < 0 ||
      column >= get_column_size()) {
    throw std::out_of_range("The row or column is out of range.");
  }
  return m_table->at(row, column);
}

connection TimeAndSalesTableModel::connect_begin_loading_signal(
    const BeginLoadingSignal::slot_type& slot) const {
  return m_begin_loading_signal.connect(slot);
}

connection TimeAndSalesTableModel::connect_end_loading_signal(
    const EndLoadingSignal::slot_type& slot) const {
  return m_end_loading_signal.connect(slot);
}

connection TimeAndSalesTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_transaction.connect_operation_signal(slot);
}

void TimeAndSalesTableModel::on_operation(const Operation& operation) {
  m_transaction.push(operation);
}
