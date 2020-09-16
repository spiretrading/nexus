#include "Spire/Ui/IntegerSpinBox.hpp"
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Spire;

IntegerSpinBox::IntegerSpinBox(std::int64_t value, QWidget* parent)
    : QAbstractSpinBox(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_spin_box = new RealSpinBox(value, this);
  m_spin_box->set_decimal_places(0);
  m_spin_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusProxy(m_spin_box);
  layout->addWidget(m_spin_box);
  m_spin_box->connect_change_signal([=] (auto value) {
    m_change_signal(value.extract_signed_long_long());
  });
  connect(m_spin_box, &RealSpinBox::editingFinished, this,
    &IntegerSpinBox::editingFinished);
}

connection IntegerSpinBox::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

void IntegerSpinBox::set_minimum(std::int64_t minimum) {
  m_spin_box->set_minimum(minimum);
}

void IntegerSpinBox::set_maximum(std::int64_t maximum) {
  m_spin_box->set_maximum(maximum);
}

std::int64_t IntegerSpinBox::get_step() const {
  return m_spin_box->get_step().extract_signed_long_long();
}

void IntegerSpinBox::set_step(std::int64_t step) {
  m_spin_box->set_step(step);
}

std::int64_t IntegerSpinBox::get_value() const {
  return m_spin_box->get_value().extract_signed_long_long();
}

void IntegerSpinBox::set_value(std::int64_t value) {
  m_spin_box->set_value(value);
}
