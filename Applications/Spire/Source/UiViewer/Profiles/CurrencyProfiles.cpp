#include "Spire/UiViewer/Profiles/CurrencyProfiles.hpp"
#include "Nexus/Definitions/StandardCurrencies.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Ui/CurrencyBox.hpp"
#include "Spire/Ui/CurrencyFilterPanel.hpp"
#include "Spire/Ui/CurrencyListBox.hpp"
#include "Spire/Ui/CurrencyListItem.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  std::shared_ptr<CurrencyEntryQueryModel> populate_currency_query_model() {
    auto model = std::make_shared<LocalQueryModel<CurrencyDatabase::Entry>>();
    for(auto& currency : CURRENCIES.get_entries()) {
      model->add(to_text(currency.m_id), currency);
      for(auto& term : QString::fromStdString(currency.m_name).split(' ')) {
        model->add(term, currency);
      }
    }
    return model;
  }
}

UiProfile Spire::make_currency_box_profile() {
  return setup_combo_box_profile("CurrencyBox", "USD",
    populate_currency_query_model(),
    [] (const CurrencyDatabase::Entry& entry) { return entry.m_id; },
    [] (auto source, auto current) {
      return make_currency_box(source, current);
    });
}

UiProfile Spire::make_currency_filter_panel_profile() {
  return setup_open_filter_panel_profile("CurrencyFilterPanel",
    [] { return make_currency_filter_panel(populate_currency_query_model()); });
}

UiProfile Spire::make_currency_list_box_profile() {
  return setup_tag_combo_box_profile("CurrencyListBox",
    [] { return make_currency_list_box(populate_currency_query_model()); });
}

UiProfile Spire::make_currency_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("CurrencyListItem", properties,
    [] (auto& profile) {
      auto item = new CurrencyListItem(CURRENCIES.from("USD"));
      apply_widget_properties(item, profile.get_properties());
      return item;
    });
  return profile;
}
