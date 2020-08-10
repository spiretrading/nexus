#include "Spire/Ui/QuantityInputWidget.hpp"
#include <QHBoxLayout>
#include <QRegularExpressionValidator>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

QuantityInputWidget::QuantityInputWidget(Nexus::Quantity value,
    QWidget* parent)
    : DecimalInputWidget(static_cast<double>(value), parent) {
  setMinimum(0);
  DecimalInputWidget::connect_change_signal([=] (auto value) {
    m_change_signal({value});
  });
  DecimalInputWidget::connect_submit_signal([=] (auto value) {
    m_submit_signal({value});
  });
}

void QuantityInputWidget::set_minimum(Nexus::Quantity minimum) {
  setMinimum(static_cast<double>(minimum));
}

void QuantityInputWidget::set_maximum(Nexus::Quantity maximum) {
  setMaximum(static_cast<double>(maximum));
}

void QuantityInputWidget::set_value(Quantity value) {
  setValue(static_cast<double>(value));
}

connection QuantityInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection QuantityInputWidget::connect_submit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}
