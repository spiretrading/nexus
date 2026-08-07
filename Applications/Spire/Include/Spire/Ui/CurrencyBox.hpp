#ifndef SPIRE_CURRENCY_BOX_HPP
#define SPIRE_CURRENCY_BOX_HPP
#include "Spire/Spire/CurrencyEntryToCurrencyQueryModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /** Represents a ValueModel over a CurrencyId. */
  using CurrencyModel = ValueModel<Nexus::CurrencyId>;

  /** Represents a LocalValueModel over a CurrencyId. */
  using LocalCurrencyModel = LocalValueModel<Nexus::CurrencyId>;

  /** A ComboBox specialized for currencies. */
  using CurrencyBox = ComboBox<Nexus::CurrencyId>;

  /**
   * Returns a new CurrencyBox using a default current model.
   * @param entries The set of currency entries that can be queried.
   * @param parent The parent widget.
   */
  CurrencyBox* make_currency_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    QWidget* parent = nullptr);

  /**
   * Returns a new CurrencyBox.
   * @param entries The set of currency entries that can be queried.
   * @param current The current currency displayed.
   * @param parent The parent widget.
   */
  CurrencyBox* make_currency_box(
    std::shared_ptr<CurrencyEntryQueryModel> entries,
    std::shared_ptr<CurrencyModel> current, QWidget* parent = nullptr);
}

#endif
