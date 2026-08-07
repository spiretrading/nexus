#include "Spire/Ui/CurrencyBox.hpp"
#include "Spire/Ui/CurrencyListItem.hpp"

using namespace Spire;

CurrencyBox* Spire::make_currency_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries, QWidget* parent) {
  return make_currency_box(
    std::move(entries), std::make_shared<LocalCurrencyModel>(), parent);
}

CurrencyBox* Spire::make_currency_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    std::shared_ptr<CurrencyModel> current, QWidget* parent) {
  return new CurrencyBox(
    std::make_shared<CurrencyEntryToCurrencyQueryModel>(entries),
    std::move(current),
    [=] (const auto& list, auto index) {
      return new CurrencyListItem(
        *entries->parse(to_text(list->get(index)).toUpper()));
    }, parent);
}
