#include "Spire/Ui/QuantitySpinBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto real_to_quantity(RealSpinBox::Real value) {
    auto quantity = Quantity::FromValue(value.str(
      std::numeric_limits<Quantity>::digits10, std::ios_base::dec));
    if(quantity != boost::none) {
      return *quantity;
    }
    throw std::runtime_error(R"(QuantitySpinBox: failed to convert Real to
      Quantity.")");
  }
}

QuantitySpinBox::QuantitySpinBox(Quantity value, QWidget* parent)
    : QAbstractSpinBox(parent) {
  m_spin_box = new RealSpinBox(display_string(value).c_str(), this);
  setFocusProxy(m_spin_box);
  m_spin_box->connect_change_signal([=] (auto value) {
    m_change_signal(real_to_quantity(value));
  });
  connect(m_spin_box, &RealSpinBox::editingFinished, this,
    &QuantitySpinBox::on_editing_finished);
  m_locale.setNumberOptions(m_locale.numberOptions().setFlag(
    QLocale::OmitGroupSeparator, true));
}

void QuantitySpinBox::resizeEvent(QResizeEvent* event) {
  m_spin_box->resize(size());
  QWidget::resizeEvent(event);
}

connection QuantitySpinBox::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

void QuantitySpinBox::set_minimum(Quantity minimum) {
  m_spin_box->set_minimum(display_string(minimum).c_str());
}

void QuantitySpinBox::set_maximum(Quantity maximum) {
  m_spin_box->set_maximum(display_string(maximum).c_str());
}

Quantity QuantitySpinBox::get_value() const {
  return real_to_quantity(m_spin_box->get_value());
}

void QuantitySpinBox::set_value(Quantity value) {
  m_spin_box->set_value(display_string(value).c_str());
}

std::string QuantitySpinBox::display_string(Quantity value) {
  return m_item_delegate.displayText(
    QVariant::fromValue(value), m_locale).toStdString();
}

void QuantitySpinBox::on_editing_finished() {
  Q_EMIT editingFinished();
}
