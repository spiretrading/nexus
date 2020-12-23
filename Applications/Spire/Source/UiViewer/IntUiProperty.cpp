#include "Spire/UiViewer/IntUiProperty.hpp"
#include <QSpinBox>

using namespace Spire;

IntUiProperty::IntUiProperty(QString name)
  : TypedUiProperty<int>(std::move(name)) {}

std::any IntUiProperty::get_value() const {
  return m_value;
}

QWidget* IntUiProperty::make_setter_widget(QWidget* parent) {
  auto setter = new QSpinBox(parent);
  QObject::connect(setter, qOverload<int>(&QSpinBox::valueChanged),
    [this] (const auto& value) {
      set(value);
    });
  return setter;
}
