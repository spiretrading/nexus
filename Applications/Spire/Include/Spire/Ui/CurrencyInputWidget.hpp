#ifndef SPIRE_CURRENCY_INPUT_WIDGET_HPP
#define SPIRE_CURRENCY_INPUT_WIDGET_HPP
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for selecting Currencies.
  class CurrencyInputWidget : public FilteredDropDownMenu {
    public:

      //! Constructs a CurrencyInputWidget.
      /*!
        \param parent The parent widget.
      */
      CurrencyInputWidget(QWidget* parent = nullptr);

      //! Returns the currently selected Currency.
      Nexus::CurrencyId get_currency() const;
  };
}

#endif
