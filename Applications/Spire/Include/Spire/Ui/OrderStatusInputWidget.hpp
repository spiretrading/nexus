#ifndef SPIRE_ORDER_STATUS_INPUT_WIDGET_HPP
#define SPIRE_ORDER_STATUS_INPUT_WIDGET_HPP
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for selecting OrderStatus'.
  class OrderStatusInputWidget : public FilteredDropDownMenu {
    public:

      //! Constructs an OrderStatusInputWidget.
      /*!
        \param parent The parent widget.
      */
      OrderStatusInputWidget(QWidget* parent = nullptr);

      //! Returns the currently selected OrderStatus.
      Nexus::OrderStatus get_order_status() const;
  };
}

#endif
