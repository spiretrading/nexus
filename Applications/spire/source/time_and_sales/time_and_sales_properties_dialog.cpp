#include "spire/time_and_sales/time_and_sales_properties_dialog.hpp"
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

time_and_sales_properties_dialog::time_and_sales_properties_dialog(
    const time_and_sales_properties& properties, QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_properties(properties) {
  setFixedSize(scale(0, 0));
}

const time_and_sales_properties&
    time_and_sales_properties_dialog::get_properties() const {
  return m_properties;
}

connection time_and_sales_properties_dialog::connect_apply_signal(
    const apply_signal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection time_and_sales_properties_dialog::connect_apply_all_signal(
    const apply_all_signal::slot_type& slot) const {
  return m_apply_all_signal.connect(slot);
}

connection time_and_sales_properties_dialog::connect_save_default_signal(
    const save_default_signal::slot_type& slot) const {
  return m_save_default_signal.connect(slot);
}
