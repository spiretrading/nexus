#ifndef SPIRE_CURRENCY_FILTER_PANEL_HPP
#define SPIRE_CURRENCY_FILTER_PANEL_HPP
#include "Spire/Ui/CurrencyListBox.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"

namespace Spire {

  /** An OpenFilterPanel specialized for a CurrencyListBox. */
  using CurrencyFilterPanel = OpenFilterPanel<CurrencyListBox>;

  /**
   * Returns a new CurrencyFilterPanel with a default current model.
   * @param entries The set of currency entries that can be queried.
   * @param parent The parent widget.
   */
  CurrencyFilterPanel* make_currency_filter_panel(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    QWidget* parent = nullptr);

  /**
   * Returns a new CurrencyFilterPanel.
   * @param entries The set of currency entries that can be queried.
   * @param current The current list of selected currencies.
   * @param parent The parent widget.
   */
  CurrencyFilterPanel* make_currency_filter_panel(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    std::shared_ptr<CurrencyListModel> current, QWidget* parent = nullptr);
}

#endif
