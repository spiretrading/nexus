#include "spire/time_and_sales/time_and_sales_window.hpp"
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"

using namespace Nexus;
using namespace spire;

time_and_sales_window::time_and_sales_window(
    const time_and_sales_properties& properties, QWidget* parent)
    : QWidget(parent) {
  set_properties(properties);
  set_model(std::make_shared<empty_time_and_sales_model>(Security()));
}

void time_and_sales_window::set_model(
    std::shared_ptr<time_and_sales_model> model) {
  m_model.emplace(std::move(model), m_properties);
}

const time_and_sales_properties& time_and_sales_window::get_properties() const {
  return m_properties;
}

void time_and_sales_window::set_properties(
    const time_and_sales_properties& properties) {}
