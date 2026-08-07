#ifndef SPIRE_CURRENCY_LIST_BOX_HPP
#define SPIRE_CURRENCY_LIST_BOX_HPP
#include "Spire/Spire/CurrencyEntryToCurrencyQueryModel.hpp"
#include "Spire/Ui/TagComboBox.hpp"

namespace Spire {

  /** The type of model used for a list of CurrencyId. */
  using CurrencyListModel = ListModel<Nexus::CurrencyId>;

  /** A TagComboBox specialized for a Nexus::CurrencyId. */
  using CurrencyListBox = TagComboBox<Nexus::CurrencyId>;

  /**
   * Returns a new CurrencyListBox using a default current model.
   * @param entries The set of currency entries that can be queried.
   * @param parent The parent widget.
   */
  CurrencyListBox* make_currency_list_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    QWidget* parent = nullptr);

  /**
   * Returns a new CurrencyListBox.
   * @param entries The set of currency entries that can be queried.
   * @param current The current list of selected currencies.
   * @param parent The parent widget.
   */
  CurrencyListBox* make_currency_list_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    std::shared_ptr<CurrencyListModel> current, QWidget* parent = nullptr);
}

#endif
