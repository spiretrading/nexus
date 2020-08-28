#ifndef SPIRE_MARKET_INPUT_WIDGET_HPP
#define SPIRE_MARKET_INPUT_WIDGET_HPP
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for selecting Markets.
  class MarketInputWidget : public FilteredDropDownMenu {
    public:

      //! Constructs a MarketInputWidget.
      /*!
        \param parent The parent widget.
      */
      MarketInputWidget(QWidget* parent = nullptr);

      //! Returns the currently selected Market.
      MarketToken get_market() const;
  };
}

#endif
