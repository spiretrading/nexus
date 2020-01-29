#include "Spire/Ui/ScalarWidget.hpp"
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Spire;

template<typename W, typename T>
ScalarWidget::ScalarWidget(W* widget, connection (W::* connector)(Signal<T>),
    std::function<void (const T&)> setter)
    : m_setter(setter) {
  m_callback_connection = connector.connect([=] (auto value) {
    on_widget_value_changed(value);
  });)
  set_layout(widget);
}

template<typename W, typename T>
ScalarWidget::ScalarWidget(W* widget, void (W::* callback)(T),
    std::function<void (const T&)> setter)
    : m_setter(setter) {
  connect(widget, callback, this, &InputWidget::on_widget_value_changed);
  set_layout(widget);
}

void ScalarWidget::set_value(Scalar value) {
  m_setter(value);
}

void ScalarWidget::set_layout(QWidget* widget) {
  setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins({});
  layout->addWidget(widget);
}

void ScalarWidget::on_widget_value_changed(Scalar value) {
  m_change_signal(value);
}
