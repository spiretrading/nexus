#include "Spire/Ui/CurrencyInputWidget.hpp"
#include <algorithm>

using namespace Nexus;
using namespace Spire;

CurrencyInputWidget::CurrencyInputWidget(QWidget* parent)
  : FilteredDropDownMenu([] {
      auto entries = GetDefaultCurrencyDatabase().GetEntries();
      auto items = [&] {
        auto currencies = std::vector<QVariant>();
        currencies.reserve(entries.size());
        std::transform(entries.begin(), entries.end(),
          std::back_inserter(currencies), [] (const auto& entry) {
            return QVariant::fromValue<CurrencyId>(entry.m_id);
          });
        return currencies;
      }();
      return items;
    }(), parent) {}

CurrencyId CurrencyInputWidget::get_currency() const {
  return get_item().value<CurrencyId>();
}
