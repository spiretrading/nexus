#include "Spire/Ui/QuantityBox.hpp"
#include <QKeyEvent>
#include "Spire/Spire/UnsignedQuantityModel.hpp"
#include "Spire/Styles/Selectors.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct QuantityToDecimalModel : ToDecimalModel<Quantity> {
    using ToDecimalModel<Quantity>::ToDecimalModel;

    optional<Decimal> get_minimum() const override {
      return Decimal(0);
    }

    optional<Decimal> get_increment() const override {
      return Decimal("0.000001");
    }
  };
}

QuantityBox::QuantityBox(QWidget* parent)
  : QuantityBox(std::make_shared<UnsignedQuantityModel>(
      std::make_shared<LocalOptionalQuantityModel>()), parent) {}

QuantityBox::QuantityBox(
  QHash<Qt::KeyboardModifier, Quantity> modifiers, QWidget* parent)
  : QuantityBox(std::make_shared<UnsignedQuantityModel>(
      std::make_shared<LocalOptionalQuantityModel>()), std::move(modifiers),
      parent) {}

QuantityBox::QuantityBox(std::shared_ptr<OptionalQuantityModel> current,
  QWidget* parent)
  : QuantityBox(std::move(current), QHash<Qt::KeyboardModifier, Type>(
    {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
    {Qt::ShiftModifier, 20}}), parent) {}

QuantityBox::QuantityBox(std::shared_ptr<OptionalQuantityModel> model,
    QHash<Qt::KeyboardModifier, Quantity> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(model, std::make_shared<QuantityToDecimalModel>(model),
        std::move(modifiers), parent) {
  update_style(get_decimal_box(), [&] (auto& style) {
    style.get(Any()).set(TrailingZeros(0));
  });
  get_decimal_box().installEventFilter(this);
}

bool QuantityBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == &get_decimal_box() && event->type() == QEvent::Show) {
    if(auto proxy = find_focus_proxy(get_decimal_box())) {
      proxy->installEventFilter(this);
    }
  } else if(event->type() == QEvent::KeyPress && !is_read_only()) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(auto current = get_current()->get()) {
      auto value = [&] {
        if(key_event.key() == Qt::Key_K) {
          return *current * 1000;
        } else if(key_event.key() == Qt::Key_H) {
          return *current * 100;
        } else if(key_event.key() == Qt::Key_D) {
          if(key_event.modifiers().testFlag(Qt::AltModifier)) {
            return *current * 10;
          } else {
            return *current / 10;
          }
        } else if(key_event.key() == Qt::Key_C) {
          return *current / 100;
        } else if(key_event.key() == Qt::Key_M) {
          return *current / 1000;
        }
        return *current;
      }();
      auto decimal_min = get_decimal_box().get_current()->get_minimum();
      auto decimal_max = get_decimal_box().get_current()->get_maximum();
      auto decimal_value = to_decimal(value);
      if(decimal_min && decimal_value <= *decimal_min) {
        value = from_decimal<Quantity>(*decimal_min);
      } else if(decimal_max && decimal_value >= *decimal_max) {
        value = from_decimal<Quantity>(*decimal_max);
      }
      if(value != *current) {
        get_current()->set(value);
      }
    }
  }
  return DecimalBoxAdaptor<Nexus::Quantity>::eventFilter(watched, event);
}
