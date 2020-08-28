#ifndef SPIRE_CURRENCY_INPUT_WIDGET_HPP
#define SPIRE_CURRENCY_INPUT_WIDGET_HPP
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for selecting currencies.
  class CurrencyInputWidget : public FilteredDropDownMenu {
    public:

      //! Constructs a CurrencyInputWidget.
      /*!
        \param parent The parent widget.
      */
      CurrencyInputWidget(QWidget* parent = nullptr);

      //! Returns the id of the currently selected currency.
      Nexus::CurrencyId get_currency() const;
  };
}

#endif
