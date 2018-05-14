#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace Nexus;
using namespace spire;

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
    case 0:
      return QVariant::fromValue(
        m_entries[row_index].m_time_and_sale.GetValue().m_timestamp);
    case 1:
      return QVariant::fromValue(
        m_entries[row_index].m_time_and_sale.GetValue().m_price);
    case 2:
      return QVariant::fromValue(
        m_entries[row_index].m_time_and_sale.GetValue().m_size);
    case 3:
      return QString::fromStdString(
        m_entries[row_index].m_time_and_sale.GetValue().m_marketCenter);
    case 4:
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
    case 0: return tr("Time");
    case 1: return tr("Price");
    case 2: return tr("Qty");
    case 3: return tr("Mkt");
    case 4: return tr("Cond");
    default: return QVariant();
  }
}

void time_and_sales_window_model::update_data(
    const time_and_sales_model::entry& e) {
  beginInsertRows(QModelIndex(), 0, 0);
  m_entries.push_back(e);
  endInsertRows();
}
