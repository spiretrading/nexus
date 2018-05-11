#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include <QDebug>

using namespace Nexus;
using namespace spire;

time_and_sales_window_model::time_and_sales_window_model(
    std::shared_ptr<time_and_sales_model> model,
    const time_and_sales_properties& properties)
    : m_model(std::move(model)) {
  set_properties(properties);
}

const Security& time_and_sales_window_model::get_security() const {
  return m_model->get_security();
}

void time_and_sales_window_model::set_properties(
    const time_and_sales_properties& properties) {}

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
  return 5;
}

QVariant time_and_sales_window_model::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role != Qt::DisplayRole) {
    return QVariant();
  }
  // Might be able to keep a vector of values, so when set_properties is called
  // I can grab the list of what columns are being displayed and assign it
  // to that vector, then when this function is called I can just return the
  // value in the vector at 'section'. I don't think I can hardcode this,
  // anyway, because the extra columns aren't just appended to the end of the
  // table, some are added/removed from the front.
  switch(section) {
    case 0: return tr("Time");
    case 1: return tr("Price");
    case 2: return tr("Qty");
    case 3: return tr("Mkt");
    case 4: return tr("Cond");
    default: return QVariant();
  }
}
