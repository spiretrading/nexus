#include "Spire/Ui/ScalarWidget.hpp"
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Spire;

Scalar ScalarWidget::get_value() const {
  return m_value;
}

void ScalarWidget::set_value(Scalar value) {
  m_value = value;
  m_setter(value);
}

connection ScalarWidget::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

void ScalarWidget::on_widget_value_changed(Scalar value) {
  m_value = value;
  m_change_signal(value);
}

void ScalarWidget::set_layout(QWidget* widget) {
  setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  layout->addWidget(widget);
}
