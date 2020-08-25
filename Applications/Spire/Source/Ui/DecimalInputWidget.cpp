#include "Spire/Ui/DecimalInputWidget.hpp"

using namespace boost::signals2;
using namespace Spire;

DecimalInputWidget::DecimalInputWidget(double value, QWidget* parent)
    : QWidget(parent) {
  m_input_widget = new NumericInputWidget(static_cast<long double>(value),
    this);
  m_input_widget->connect_change_signal([=] (auto value) {
    m_change_signal(value.extract_double());
  });
  connect(m_input_widget, &NumericInputWidget::editingFinished, this,
    &DecimalInputWidget::on_editing_finished);
}

void DecimalInputWidget::resizeEvent(QResizeEvent* event) {
  m_input_widget->resize(size());
  QWidget::resizeEvent(event);
}

connection DecimalInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection DecimalInputWidget::connect_commit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void DecimalInputWidget::set_minimum(double minimum) {
  m_input_widget->set_minimum(static_cast<long double>(minimum));
}

void DecimalInputWidget::set_maximum(double maximum) {
  m_input_widget->set_maximum(static_cast<long double>(maximum));
}

double DecimalInputWidget::get_value() const {
  return m_input_widget->get_value().extract_double();
}

void DecimalInputWidget::set_value(double value) {
  m_input_widget->set_value(static_cast<long double>(value));
}

void DecimalInputWidget::on_editing_finished() {
  m_commit_signal(m_input_widget->get_value().extract_double());
}
