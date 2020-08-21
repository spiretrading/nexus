#include "Spire/Ui/QuantityInputWidget.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto real_to_quantity(NumericInputWidget::Real value) {
    auto quantity = Quantity::FromValue(value.str(
      std::numeric_limits<Quantity>::digits10, std::ios_base::dec));
    if(quantity != boost::none) {
      return *quantity;
    }
    return Quantity();
  }
}

QuantityInputWidget::QuantityInputWidget(Quantity value, QWidget* parent)
    : QWidget(parent) {
  m_input_widget = new NumericInputWidget(display_string(value).c_str(), this);
  m_input_widget->connect_change_signal([=] (auto value) {
    m_change_signal(real_to_quantity(value));
  });
  connect(m_input_widget, &NumericInputWidget::editingFinished, this,
    &QuantityInputWidget::on_editing_finished);
  m_locale.setNumberOptions(m_locale.numberOptions().setFlag(
    QLocale::OmitGroupSeparator, true));
}

void QuantityInputWidget::resizeEvent(QResizeEvent* event) {
  m_input_widget->resize(size());
  QWidget::resizeEvent(event);
}

connection QuantityInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection QuantityInputWidget::connect_commit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void QuantityInputWidget::set_minimum(Quantity minimum) {
  m_input_widget->set_minimum(display_string(minimum).c_str());
}

void QuantityInputWidget::set_maximum(Quantity maximum) {
  m_input_widget->set_maximum(display_string(maximum).c_str());
}

Quantity QuantityInputWidget::get_value() const {
  return real_to_quantity(m_input_widget->get_value());
}

void QuantityInputWidget::set_value(Quantity value) {
  m_input_widget->set_value(display_string(value).c_str());
}

std::string QuantityInputWidget::display_string(Quantity value) {
  return m_item_delegate.displayText(
    QVariant::fromValue(value), m_locale).toStdString();
}

void QuantityInputWidget::on_editing_finished() {
  m_commit_signal(real_to_quantity(m_input_widget->get_value()));
}
