#include "Spire/UiViewer/StandardUiProperties.hpp"
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>

using namespace Spire;

std::shared_ptr<TypedUiProperty<bool>> Spire::make_standard_bool_property(
    QString name) {
  return make_standard_bool_property(std::move(name), false);
}

std::shared_ptr<TypedUiProperty<bool>> Spire::make_standard_bool_property(
    QString name, bool value) {
  return std::make_shared<StandardUiProperty<bool>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<bool>& property) {
      auto setter = new QCheckBox(parent);
      if(property.get()) {
        setter->setCheckState(Qt::Checked);
      } else {
        setter->setCheckState(Qt::Unchecked);
      }
      QObject::connect(setter, &QCheckBox::stateChanged,
        [&property] (auto value) {
          if(value == Qt::Checked) {
            property.set(true);
          } else {
            property.set(false);
          }
        });
      return setter;
    });
}

std::shared_ptr<TypedUiProperty<int>> Spire::make_standard_int_property(
    QString name) {
  return make_standard_int_property(std::move(name), 0);
}

std::shared_ptr<TypedUiProperty<int>> Spire::make_standard_int_property(
    QString name, int value) {
  return std::make_shared<StandardUiProperty<int>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<int>& property) {
      auto setter = new QSpinBox(parent);
      setter->setMinimum(1);
      setter->setMaximum(100000000);
      setter->setValue(property.get());
      QObject::connect(setter, qOverload<int>(&QSpinBox::valueChanged),
        [&property] (auto value) {
          property.set(value);
        });
      return setter;
    });
}

std::shared_ptr<TypedUiProperty<QString>> Spire::make_standard_qstring_property(
    QString name) {
  return make_standard_qstring_property(std::move(name), QString());
}

std::shared_ptr<TypedUiProperty<QString>> Spire::make_standard_qstring_property(
    QString name, QString value) {
  return std::make_shared<StandardUiProperty<QString>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<QString>& property) {
      auto setter = new QTextEdit(property.get(), parent);
      QObject::connect(setter, &QTextEdit::textChanged,
        [=, &property] {
          property.set(setter->toPlainText());
        });
      return setter;
    });
}
