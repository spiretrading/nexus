#include "Spire/Ui/QuantityBox.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/UnsignedQuantityModel.hpp"

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

QuantityBox::QuantityBox(QHash<Qt::KeyboardModifier, Quantity> modifiers,
  QWidget* parent)
  : QuantityBox(std::make_shared<UnsignedQuantityModel>(
        std::make_shared<LocalOptionalQuantityModel>()),
      std::move(modifiers), parent) {}

QuantityBox::QuantityBox(std::shared_ptr<OptionalQuantityModel> model,
    QHash<Qt::KeyboardModifier, Quantity> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(model, std::make_shared<QuantityToDecimalModel>(model),
        std::move(modifiers), parent) {
  update_style(get_decimal_box(), [&] (auto& style) {
    style.get(Any()).set(TrailingZeros(0));
  });
}
