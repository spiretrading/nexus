#include "Spire/Ui/IntegerInputWidget.hpp"
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Spire;

IntegerInputWidget::IntegerInputWidget(std::uint64_t value, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_input_widget = new NumericInputWidget(value, this);
  layout->addWidget(m_input_widget);
  m_input_widget->connect_change_signal([=] (auto value) {
    m_change_signal(value.extract_unsigned_long_long());
  });
}

void IntegerInputWidget::resizeEvent(QResizeEvent* event) {
  m_input_widget->resize(size());
  QWidget::resizeEvent(event);
}

connection IntegerInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection IntegerInputWidget::connect_commit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

void IntegerInputWidget::set_minimum(std::uint64_t minimum) {
  m_input_widget->set_minimum(minimum);
}

void IntegerInputWidget::set_maximum(std::uint64_t maximum) {
  m_input_widget->set_maximum(maximum);
}

std::uint64_t IntegerInputWidget::get_value() const {
  return m_input_widget->get_value().extract_unsigned_long_long();
}

void IntegerInputWidget::set_value(std::uint64_t value) {
  m_input_widget->set_value(value);
}

void IntegerInputWidget::on_editing_finished() {
  m_commit_signal(m_input_widget->get_value().extract_unsigned_long_long());
}
