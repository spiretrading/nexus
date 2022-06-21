#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct MoneyToDecimalModel : ToDecimalModel<Money> {
    using ToDecimalModel<Money>::ToDecimalModel;

    optional<Decimal> get_increment() const override {
      return Decimal("0.000001");
    }
  };
}

MoneyBox::MoneyBox(QWidget* parent)
  : MoneyBox(QHash<Qt::KeyboardModifier, Type>(
      {{Qt::NoModifier, Money::CENT}, {Qt::AltModifier, 5 * Money::CENT},
      {Qt::ControlModifier, 10 * Money::CENT},
      {Qt::ShiftModifier, 20 * Money::CENT}}), parent) {}

MoneyBox::MoneyBox(QHash<Qt::KeyboardModifier, Money> modifiers,
  QWidget* parent)
  : MoneyBox(std::make_shared<LocalOptionalMoneyModel>(), std::move(modifiers),
      parent) {}

MoneyBox::MoneyBox(std::shared_ptr<OptionalMoneyModel> model,
    QHash<Qt::KeyboardModifier, Money> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(model, std::make_shared<MoneyToDecimalModel>(model),
        std::move(modifiers), parent) {
  update_style(get_decimal_box(), [&] (auto& style) {
    style.get(Any()).set(TrailingZeros(2));
  });
}

MoneyBox* Spire::make_money_label(
    std::shared_ptr<MoneyModel> current, QWidget* parent) {
  auto label = new MoneyBox(std::make_shared<
    OptionalScalarValueModelDecorator<Money>>(current), {}, parent);
  apply_label_style(*label);
  label->set_read_only(true);
  return label;
}
