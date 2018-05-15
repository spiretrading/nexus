#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"

using namespace Nexus;
using namespace spire;
using columns = time_and_sales_properties::columns;
using price_range = time_and_sales_properties::price_range;

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
  if(!index.isValid()) {
    return QVariant();
  }
  auto row_index = (m_entries.size() - index.row()) - 1;
  if(role == Qt::DisplayRole) {
    switch(static_cast<columns>(index.column())) {
      case columns::TIME_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_timestamp);
      case columns::PRICE_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_price);
      case columns::SIZE_COLUMN:
        return QVariant::fromValue(
          m_entries[row_index].m_time_and_sale.GetValue().m_size);
      case columns::MARKET_COLUMN:
        return QString::fromStdString(
          m_entries[row_index].m_time_and_sale.GetValue().m_marketCenter);
      case columns::CONDITION_COLUMN:
        return QString::fromStdString(
          m_entries[row_index].m_time_and_sale.GetValue().m_condition.m_code);
      default:
        return QVariant();
    }
  } else if(role == Qt::BackgroundRole) {
    //return QBrush(m_properties.get_band_color(m_entries[row_index].m_price_range));
    return QColor(255, 0, 0);
  } else if(role == Qt::ForegroundRole) {
    return m_properties.get_text_color(m_entries[row_index].m_price_range);
  } else if(role == Qt::FontRole) {
    return m_properties.m_font;
  }
  return QVariant();
}

QVariant time_and_sales_window_model::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::DisplayRole) {
    switch(static_cast<columns>(section)) {
      case columns::TIME_COLUMN:
        return tr("Time");
      case columns::PRICE_COLUMN:
        return tr("Price");
      case columns::SIZE_COLUMN:
        return tr("Qty");
      case columns::MARKET_COLUMN:
        return tr("Mkt");
      case columns::CONDITION_COLUMN:
        return tr("Cond");
      default:
        return QVariant();
    }
  }
  return QVariant();
}

void time_and_sales_window_model::update_data(
    const time_and_sales_model::entry& e) {
  beginInsertRows(QModelIndex(), 0, 0);
  m_entries.push_back(e);
  endInsertRows();
}
