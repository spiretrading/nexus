#include "Spire/Ui/QuantityBox.hpp"

using namespace boost;
using namespace signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct QuantityToDecimalModel : ToDecimalModel<Quantity> {
    using ToDecimalModel<Quantity>::ToDecimalModel;

    Decimal get_increment() const override {
      return Decimal("0.000001");
    }
  };
}

struct QuantityBox::UnsignedQuantityModel : OptionalQuantityModel {
  mutable CurrentSignal m_current_signal;
  std::shared_ptr<OptionalQuantityModel> m_model;
  scoped_connection m_current_connection;

  UnsignedQuantityModel(std::shared_ptr<OptionalQuantityModel> model)
    : m_model(std::move(model)),
      m_current_connection(m_model->connect_current_signal(
        [=] (const auto& current) {
          on_current(current);
        })) {}

  optional<Quantity> get_minimum() const override {
    return 0;
  }

  optional<Quantity> get_maximum() const override {
    return m_model->get_maximum();
  }

  Quantity get_increment() const override {
    return m_model->get_increment();
  }

  QValidator::State get_state() const override {
    return m_model->get_state();
  }

  const optional<Quantity>& get_current() const override {
    return m_model->get_current();
  }

  QValidator::State set_current(const optional<Quantity>& value) override {
    if(value && *value < 0) {
      return m_model->set_current(Quantity(0));
    }
    return m_model->set_current(value);
  }

  connection connect_current_signal(
      const CurrentSignal::slot_type& slot) const override {
    return m_current_signal.connect(slot);
  }

  void on_current(const optional<Quantity>& current) {
    m_current_signal(current);
  }
};

QuantityBox::QuantityBox(QHash<Qt::KeyboardModifier, Quantity> modifiers,
  QWidget* parent)
  : QuantityBox(std::make_shared<LocalOptionalQuantityModel>(),
      std::move(modifiers), parent) {}

QuantityBox::QuantityBox(std::shared_ptr<OptionalQuantityModel> model,
    QHash<Qt::KeyboardModifier, Quantity> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(std::make_shared<UnsignedQuantityModel>(model),
        std::make_shared<QuantityToDecimalModel>(
          std::make_shared<UnsignedQuantityModel>(model)),
        std::move(modifiers), parent) {
  auto style = get_style(get_decimal_box());
  style.get(Any()).set(TrailingZeros(0));
  set_style(get_decimal_box(), std::move(style));
}
