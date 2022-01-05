#include "Spire/Ui/StandardTableFilter.hpp"
#include "Spire/Ui/ScalarFilterPanel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

StandardTableFilter::StandardTableFilter(std::vector<std::type_index> types)
  : m_types(std::move(types)) {}

TableFilter::Filter StandardTableFilter::get_filter(int column) {
  if(m_types[column] == typeid(void)) {
    return Filter::NONE;
  }
  return Filter::UNFILTERED;
}

QWidget* StandardTableFilter::make_filter_widget(int column, QWidget& parent) {
  if(m_types[column] == typeid(Quantity)) {
    return new QuantityFilterPanel(none, none, "Filter Quantity", parent);
  }
  return nullptr;
}

bool StandardTableFilter::is_filtered(const TableModel& model, int row) const {
  return false;
}
