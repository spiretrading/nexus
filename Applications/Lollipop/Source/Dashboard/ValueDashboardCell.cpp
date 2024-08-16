#include "Spire/Dashboard/ValueDashboardCell.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ValueDashboardCell::ValueDashboardCell()
    : m_values(30) {}

void ValueDashboardCell::SetBufferSize(int size) {
  m_values.set_capacity(size);
}

void ValueDashboardCell::SetValue(const Value& value) {
  m_values.push_back(value);
  m_updateSignal(value);
}

const circular_buffer<ValueDashboardCell::Value>&
    ValueDashboardCell::GetValues() const {
  return m_values;
}

connection ValueDashboardCell::ConnectUpdateSignal(
    const UpdateSignal::slot_function_type& slot) const {
  return m_updateSignal.connect(slot);
}
