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
  auto quantity = m_item_delegate.displayText(
    QVariant::fromValue<Quantity>(minimum));
  m_input_widget->setMinimum(value_from_text(quantity));
}

void QuantityInputWidget::set_maximum(Nexus::Quantity maximum) {
  auto quantity = m_item_delegate.displayText(
    QVariant::fromValue<Quantity>(maximum));
  m_input_widget->setMaximum(value_from_text(quantity));
}

void QuantityInputWidget::set_value(Quantity value) {
  auto quantity = m_item_delegate.displayText(
    QVariant::fromValue<Quantity>(value));
  m_input_widget->setValue(value_from_text(quantity));
}

connection QuantityInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection QuantityInputWidget::connect_submit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

double QuantityInputWidget::value_from_text(const QString& text) {
  auto min = m_input_widget->minimum();
  auto max = m_input_widget->maximum();
  m_input_widget->setMinimum(std::numeric_limits<double>::lowest());
  m_input_widget->setMaximum(std::numeric_limits<double>::max());
  auto value = m_input_widget->valueFromText(text);
  m_input_widget->setMinimum(min);
  m_input_widget->setMaximum(max);
  return value;
}

void QuantityInputWidget::on_editing_finished() {
  auto quantity = Quantity::FromValue(m_input_widget->text().toStdString());
  if(quantity) {
    m_submit_signal(*quantity);
  }
}

void QuantityInputWidget::on_value_changed() {
  auto quantity = Quantity::FromValue(m_input_widget->text().toStdString());
  if(quantity) {
    m_change_signal(*quantity);
  }
}
