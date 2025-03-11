#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Spire/OptionalScalarValueModelDecorator.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::multiprecision;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto MAX_MONEY = pow(Decimal(10),
    std::numeric_limits<Money>::digits10) - 1;
  const auto MIN_MONEY = Decimal(-MAX_MONEY);

  struct MoneyToDecimalModel : ToDecimalModel<Money> {
    using ToDecimalModel<Money>::ToDecimalModel;

    optional<Decimal> get_minimum() const override {
      if(auto min = ToDecimalModel<Money>::get_minimum()) {
        if(*min < MIN_MONEY) {
          return MIN_MONEY;
        }
        return min;
      }
      return MIN_MONEY;
    }

    optional<Decimal> get_maximum() const override {
      if(auto max = ToDecimalModel<Money>::get_maximum()) {
        if(*max > MAX_MONEY) {
          return MAX_MONEY;
        }
        return max;
      }
      return MAX_MONEY;
    }

    optional<Decimal> get_increment() const override {
      return Decimal("0.000001");
    }
  };
}

MoneyBox::MoneyBox(QWidget* parent)
  : MoneyBox(std::make_shared<LocalOptionalMoneyModel>(), parent) {}

MoneyBox::MoneyBox(std::shared_ptr<OptionalMoneyModel> current, QWidget* parent)
  : MoneyBox(std::move(current), QHash<Qt::KeyboardModifier, Money>(
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
