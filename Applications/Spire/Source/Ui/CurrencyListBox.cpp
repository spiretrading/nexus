#include "Spire/Ui/CurrencyListBox.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/CurrencyListItem.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

CurrencyListBox* Spire::make_currency_list_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries, QWidget* parent) {
  return make_currency_list_box(std::move(entries),
    std::make_shared<ArrayListModel<CurrencyId>>(), parent);
}

CurrencyListBox* Spire::make_currency_list_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    std::shared_ptr<CurrencyListModel> current, QWidget* parent) {
  return new CurrencyListBox(
    std::make_shared<CurrencyEntryToCurrencyQueryModel>(entries),
    std::move(current),
    [=] (const std::shared_ptr<CurrencyListModel>& list, int index) {
      if(auto entry = entries->parse(to_text(list->get(index)))) {
        return new CurrencyListItem(*entry);
      }
      return new CurrencyListItem(CurrencyDatabase::Entry());
    }, parent);
}
