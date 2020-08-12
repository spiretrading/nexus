#include "Spire/Ui/QuantityInputWidget.hpp"
#include <QHBoxLayout>
#include <QLineEdit>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

QuantityInputWidget::QuantityInputWidget(Nexus::Quantity value,
    QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_input_widget = new DecimalInputWidget(0, this);
  layout->addWidget(m_input_widget);
  setFocusProxy(m_input_widget);
  connect(m_input_widget, &DecimalInputWidget::editingFinished,
    this, &QuantityInputWidget::on_editing_finished);
  connect(m_input_widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    [=] (auto value) { on_value_changed(); });
}

void QuantityInputWidget::resizeEvent(QResizeEvent* event) {
  m_input_widget->setFixedSize(size());
  QWidget::resizeEvent(event);
}

void QuantityInputWidget::set_minimum(Nexus::Quantity minimum) {
  m_input_widget->setMinimum(static_cast<double>(minimum));
}

void QuantityInputWidget::set_maximum(Nexus::Quantity maximum) {
  m_input_widget->setMaximum(static_cast<double>(maximum));
}

void QuantityInputWidget::set_value(Quantity value) {
  m_input_widget->setValue(static_cast<double>(value));
}

connection QuantityInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection QuantityInputWidget::connect_submit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void QuantityInputWidget::on_editing_finished() {
  m_submit_signal(*Quantity::FromValue(m_input_widget->text().toStdString()));
}

void QuantityInputWidget::on_value_changed() {
  m_change_signal(*Quantity::FromValue(m_input_widget->text().toStdString()));
}
