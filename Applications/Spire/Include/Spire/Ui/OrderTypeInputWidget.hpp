#ifndef SPIRE_ORDER_TYPE_INPUT_WIDGET_HPP
#define SPIRE_ORDER_TYPE_INPUT_WIDGET_HPP
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for selecting OrderTypes.
  class OrderTypeInputWidget : public FilteredDropDownMenu {
    public:

      //! Constructs an OrderTypeInputWidget.
      /*!
        \param parent The parent widget.
      */
      OrderTypeInputWidget(QWidget* parent = nullptr);

      //! Returns the currently selected OrderType.
      Nexus::OrderType get_order_type() const;
  };
}

#endif
