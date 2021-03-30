#include "Spire/UiViewer/StandardUiProperties.hpp"
#include <limits>
#include <QCheckBox>
#include <QLineEdit>
#include <QResizeEvent>
#include <QSpinBox>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  struct SizeFilter : QObject {
    TypedUiProperty<int>* m_width;
    TypedUiProperty<int>* m_height;

    SizeFilter(TypedUiProperty<int>* width, TypedUiProperty<int>* height,
      QObject* parent)
      : QObject(parent),
        m_width(width),
        m_height(height) {}

    bool eventFilter(QObject* object, QEvent* event) override {
      if(event->type() == QEvent::Resize) {
        auto& resizeEvent = static_cast<QResizeEvent&>(*event);
        if(scale_width(m_width->get()) != resizeEvent.size().width()) {
          m_width->set(unscale_width(resizeEvent.size().width()));
        }
        if(scale_height(m_height->get()) != resizeEvent.size().height()) {
          m_height->set(unscale_height(resizeEvent.size().height()));
        }
      }
      return QObject::eventFilter(object, event);
    };
  };
}

void Spire::populate_widget_properties(
    std::vector<std::shared_ptr<UiProperty>>& properties) {
  properties.push_back(make_standard_bool_property("enabled", true));
  properties.push_back(make_standard_int_property("width"));
  properties.push_back(make_standard_int_property("height"));
}

void Spire::apply_widget_properties(QWidget* widget,
    const std::vector<std::shared_ptr<UiProperty>>& properties) {
  auto& enabled = get<bool>("enabled", properties);
  auto& width = get<int>("width", properties);
  auto& height = get<int>("height", properties);
  enabled.connect_changed_signal([=] (auto value) {
    widget->setEnabled(value);
  });
  width.connect_changed_signal([=] (auto value) {
    if(value != 0) {
      if(unscale_width(widget->width()) != value) {
        widget->setFixedWidth(scale_width(value));
      }
    }
  });
  height.connect_changed_signal([=] (auto value) {
    if(value != 0) {
      if(unscale_height(widget->height()) != value) {
        widget->setFixedHeight(scale_height(value));
      }
    }
  });
  widget->installEventFilter(new SizeFilter(&width, &height, widget));
}

std::shared_ptr<TypedUiProperty<bool>> Spire::make_standard_bool_property(
    QString name) {
  return make_standard_bool_property(std::move(name), false);
}

std::shared_ptr<TypedUiProperty<bool>> Spire::make_standard_bool_property(
    QString name, bool value) {
  return std::make_shared<StandardUiProperty<bool>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<bool>& property) {
      auto setter = new QCheckBox(parent);
      property.connect_changed_signal([=] (auto value) {
        if(value) {
          setter->setCheckState(Qt::Checked);
        } else {
          setter->setCheckState(Qt::Unchecked);
        }
      });
      QObject::connect(setter, &QCheckBox::stateChanged, [&] (auto value) {
        property.set(value == Qt::Checked);
      });
      return setter;
    });
}

std::shared_ptr<TypedUiProperty<CurrencyId>>
    Spire::make_standard_currency_property(QString name) {
  return make_standard_currency_property(std::move(name), CurrencyId::NONE);
}

std::shared_ptr<TypedUiProperty<CurrencyId>>
    Spire::make_standard_currency_property(QString name, CurrencyId value) {
  return std::make_shared<StandardUiProperty<CurrencyId>>(std::move(name),
    value,
    [] (QWidget* parent, StandardUiProperty<CurrencyId>& property) {
      auto setter = new QLineEdit(parent);
      property.connect_changed_signal([=] (auto value) {
        auto code = GetDefaultCurrencyDatabase().FromId(value).m_code;
        setter->setText(QString::fromStdString(code.GetData()));
      });
      QObject::connect(setter, &QLineEdit::textChanged, [&] (const auto& text) {
        auto id = GetDefaultCurrencyDatabase().FromCode(
          text.toUpper().toStdString()).m_id;
        if(id != CurrencyId::NONE) {
          property.set(id);
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
      setter->setMinimum(std::numeric_limits<int>::min());
      setter->setMaximum(std::numeric_limits<int>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(value);
      });
      QObject::connect(setter, qOverload<int>(&QSpinBox::valueChanged),
        [&] (auto value) {
          property.set(value);
        });
      return setter;
    });
}

std::shared_ptr<TypedUiProperty<std::int64_t>>
    Spire::make_standard_int64_property(QString name, std::int64_t value) {
  return std::make_shared<StandardUiProperty<std::int64_t>>(std::move(name),
    value,
    [] (QWidget* parent, StandardUiProperty<std::int64_t>& property) {
      auto setter = new QSpinBox(parent);
      setter->setMinimum(std::numeric_limits<int>::min());
      setter->setMaximum(std::numeric_limits<int>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<int>(value));
      });
      QObject::connect(setter, qOverload<int>(&QSpinBox::valueChanged),
        [&] (auto value) {
          property.set(value);
        });
      return setter;
    });
}

std::shared_ptr<TypedUiProperty<QColor>> Spire::make_standard_qcolor_property(
    QString name) {
  return make_standard_qcolor_property(std::move(name), QColorConstants::White);
}

std::shared_ptr<TypedUiProperty<QColor>> Spire::make_standard_qcolor_property(
    QString name, QColor value) {
  return std::make_shared<StandardUiProperty<QColor>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<QColor>& property) {
      auto setter = new QLineEdit(property.get().name().toUpper(), parent);
      property.connect_changed_signal([=] (auto value) {
        setter->setText(value.name());
      });
      QObject::connect(setter, &QLineEdit::textChanged,
        [&] (const auto& text) {
          if(text.length() == 7) {
            auto color = QColor(text.toUpper());
            if(color.isValid()) {
              property.set(color);
            }
          }
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
      auto setter = new QLineEdit(property.get(), parent);
      property.connect_changed_signal([=] (const auto& value) {
        setter->setText(value);
      });
      QObject::connect(setter, &QLineEdit::textChanged, [&] (const auto& text) {
        property.set(text);
      });
      return setter;
    });
}
