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
using namespace Spire::Styles;

namespace {
  struct SizeFilter : QObject {
    TypedUiProperty<int>* m_width;
    TypedUiProperty<int>* m_height;

    SizeFilter(TypedUiProperty<int>* width,
      TypedUiProperty<int>* height, QObject* parent)
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
    }
  };
}

void Spire::populate_widget_size_properties(const QString& width_name,
    const QString& height_name,
    std::vector<std::shared_ptr<UiProperty>>& properties) {
  properties.push_back(make_standard_property<int>(width_name));
  properties.push_back(make_standard_property<int>(height_name));
}

void Spire::apply_widget_size_properties(QWidget* widget,
    const QString& width_name, const QString& height_name,
    const std::vector<std::shared_ptr<UiProperty>>& properties) {
  auto& width = get<int>(width_name, properties);
  auto& height = get<int>(height_name, properties);
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

void Spire::populate_widget_properties(
    std::vector<std::shared_ptr<UiProperty>>& properties) {
  properties.push_back(make_standard_property("enabled", true));
  populate_widget_size_properties("width", "height", properties);
}

void Spire::apply_widget_properties(QWidget* widget,
    const std::vector<std::shared_ptr<UiProperty>>& properties) {
  auto& enabled = get<bool>("enabled", properties);
  enabled.connect_changed_signal([=] (auto value) {
    widget->setEnabled(value);
  });
  apply_widget_size_properties(widget, "width", "height", properties);
}

template<>
std::shared_ptr<TypedUiProperty<bool>> Spire::make_standard_property<bool>(
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

template<>
std::shared_ptr<TypedUiProperty<CurrencyId>>
    Spire::make_standard_property<CurrencyId>(QString name, CurrencyId value) {
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

template<>
std::shared_ptr<TypedUiProperty<int>> Spire::make_standard_property<int>(
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

template<>
std::shared_ptr<TypedUiProperty<std::int64_t>>
    Spire::make_standard_property<std::int64_t>(
      QString name, std::int64_t value) {
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

template<>
std::shared_ptr<TypedUiProperty<Money>> Spire::make_standard_property<Money>(
    QString name, Money value) {
  return std::make_shared<StandardUiProperty<Money>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<Money>& property) {
      auto setter = new QDoubleSpinBox(parent);
      setter->setMinimum(std::numeric_limits<double>::lowest());
      setter->setMaximum(std::numeric_limits<double>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<double>(value));
      });
      QObject::connect(setter, &QDoubleSpinBox::textChanged,
        [&] (const auto& value) {
          if(auto money = Money::FromValue(value.toStdString())) {
            property.set(*money);
          }
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<Quantity>>
    Spire::make_standard_property<Quantity>(QString name, Quantity value) {
  return std::make_shared<StandardUiProperty<Quantity>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<Quantity>& property) {
      auto setter = new QDoubleSpinBox(parent);
      setter->setMinimum(std::numeric_limits<double>::lowest());
      setter->setMaximum(std::numeric_limits<double>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<double>(value));
      });
      QObject::connect(setter, &QDoubleSpinBox::textChanged,
        [&] (const auto& value) {
          if(auto quantity = Quantity::FromValue(value.toStdString())) {
            property.set(*quantity);
          }
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<Decimal>>
    Spire::make_standard_property<Decimal>(QString name, Decimal value) {
  return std::make_shared<StandardUiProperty<Decimal>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<Decimal>& property) {
      auto setter = new QDoubleSpinBox(parent);
      setter->setMinimum(std::numeric_limits<double>::lowest());
      setter->setMaximum(std::numeric_limits<double>::max());
      auto parse_decimal = [] (auto decimal) -> std::optional<Decimal> {
        try {
          return Decimal(decimal.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<double>(value));
      });
      QObject::connect(setter, &QDoubleSpinBox::textChanged,
        [&] (const auto& value) {
          if(auto decimal = parse_decimal(value)) {
            property.set(*decimal);
          }
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<QColor>>
    Spire::make_standard_property<QColor>(QString name) {
  return make_standard_property(std::move(name), QColorConstants::White);
}

template<>
std::shared_ptr<TypedUiProperty<QColor>>
    Spire::make_standard_property<QColor>(QString name, QColor value) {
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

template<>
std::shared_ptr<TypedUiProperty<QString>>
    Spire::make_standard_property<QString>(QString name, QString value) {
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

template<>
std::shared_ptr<TypedUiProperty<DateFormat>>
    Spire::make_standard_property<DateFormat>(QString name, DateFormat value) {
  auto formats = define_enum<DateFormat>(
    {{"YYYYMMDD", DateFormat::YYYYMMDD}, {"MMDD", DateFormat::MMDD}});
  return make_standard_enum_property(std::move(name), value, formats);
}

template<>
std::shared_ptr<TypedUiProperty<DateFormat>>
    Spire::make_standard_property<DateFormat>(QString name) {
  return make_standard_property(std::move(name), DateFormat::YYYYMMDD);
}
