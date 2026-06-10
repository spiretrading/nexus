#include "Spire/Ui/CurrencyFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace Nexus;
using namespace Spire;

CurrencyFilterPanel* Spire::make_currency_filter_panel(
    std::shared_ptr<CurrencyEntryQueryModel> entries, QWidget* parent) {
  return make_currency_filter_panel(std::move(entries),
    std::make_shared<ArrayListModel<CurrencyId>>(), parent);
}

CurrencyFilterPanel* Spire::make_currency_filter_panel(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    std::shared_ptr<CurrencyListModel> current, QWidget* parent) {
  auto box = make_currency_list_box(std::move(entries), std::move(current));
  box->set_placeholder(QObject::tr("Enter currencies"));
  return new OpenFilterPanel(*box, parent);
}
