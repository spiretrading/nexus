#include "spire/time_and_sales/time_and_sales_window_model.hpp"

using namespace spire;

time_and_sales_window_model::time_and_sales_window_model(
    std::shared_ptr<time_and_sales_model> model,
    const time_and_sales_properties& properties)
    : m_model(std::move(model)) {
  set_properties(properties);
}

void time_and_sales_window_model::set_properties(
    const time_and_sales_properties& properties) {}

int time_and_sales_window_model::rowCount(const QModelIndex& parent) const {
  return {};
}

int time_and_sales_window_model::columnCount(const QModelIndex& parent) const {
  return {};
}

QVariant time_and_sales_window_model::data(const QModelIndex& index,
    int role) const {
  return {};
}

QVariant time_and_sales_window_model::headerData(int section,
    Qt::Orientation orientation, int role) const {
  return {};
}
