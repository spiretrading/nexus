#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace Nexus;
using namespace spire;
using columns = time_and_sales_properties::columns;

time_and_sales_window_model::time_and_sales_window_model(
    std::shared_ptr<time_and_sales_model> model,
    const time_and_sales_properties& properties)
    : m_model(std::move(model)) {
  set_properties(properties);
  m_model.get()->connect_time_and_sale_signal(
    [=] (auto e) { update_data(e); });
}

const Security& time_and_sales_window_model::get_security() const {
  return m_model->get_security();
}

void time_and_sales_window_model::set_properties(
    const time_and_sales_properties& properties) {
  m_properties = properties;
}

int time_and_sales_window_model::rowCount(const QModelIndex& parent) const {
  return m_entries.size();
}

int time_and_sales_window_model::columnCount(const QModelIndex& parent) const {
  return m_properties.m_show_columns.size();
}

QVariant time_and_sales_window_model::data(const QModelIndex& index,
    int role) const {
  if(role != Qt::DisplayRole || !index.isValid()) {
    return QVariant();
  }
  auto row_index = (m_entries.size() - index.row()) - 1;
  switch(index.column()) {
    case static_cast<int>(columns::TIME_COLUMN):
      return QVariant::fromValue(
        m_entries[row_index].m_time_and_sale.GetValue().m_timestamp);
    case static_cast<int>(columns::PRICE_COLUMN):
      return QVariant::fromValue(
        m_entries[row_index].m_time_and_sale.GetValue().m_price);
    case static_cast<int>(columns::SIZE_COLUMN):
      return QVariant::fromValue(
        m_entries[row_index].m_time_and_sale.GetValue().m_size);
    case static_cast<int>(columns::MARKET_COLUMN):
      return QString::fromStdString(
        m_entries[row_index].m_time_and_sale.GetValue().m_marketCenter);
    case static_cast<int>(columns::CONDITION_COLUMN):
      return QString::fromStdString(
        m_entries[row_index].m_time_and_sale.GetValue().m_condition.m_code);
    default:
      return QVariant();
  }
}

QVariant time_and_sales_window_model::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role != Qt::DisplayRole) {
    return QVariant();
  }
  switch(section) {
    case static_cast<int>(columns::TIME_COLUMN):
      return tr("Time");
    case static_cast<int>(columns::PRICE_COLUMN):
      return tr("Price");
    case static_cast<int>(columns::SIZE_COLUMN):
      return tr("Qty");
    case static_cast<int>(columns::MARKET_COLUMN):
      return tr("Mkt");
    case static_cast<int>(columns::CONDITION_COLUMN):
      return tr("Cond");
    default:
      return QVariant();
  }
}

void time_and_sales_window_model::update_data(
    const time_and_sales_model::entry& e) {
  beginInsertRows(QModelIndex(), 0, 0);
  m_entries.push_back(e);
  endInsertRows();
}
