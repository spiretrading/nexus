#include "Spire/Ui/MoneyBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct MoneyToDecimalModel : ToDecimalModel<Money> {
    using ToDecimalModel<Money>::ToDecimalModel;

    Decimal get_increment() const override {
      return Decimal("0.000001");
    }
  };
}

MoneyBox::MoneyBox(QHash<Qt::KeyboardModifier, Money> modifiers,
  QWidget* parent)
  : MoneyBox(std::make_shared<LocalOptionalMoneyModel>(),
      std::move(modifiers), parent) {}

MoneyBox::MoneyBox(std::shared_ptr<OptionalMoneyModel> model,
    QHash<Qt::KeyboardModifier, Money> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(model, std::make_shared<MoneyToDecimalModel>(model),
        std::move(modifiers), parent) {
  auto style = get_style(get_decimal_box());
  style.get(Any()).set(TrailingZeros(2));
  set_style(get_decimal_box(), std::move(style));
}
