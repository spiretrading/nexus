#ifndef SPIRE_TIME_IN_FORCE_INPUT_WIDGET_HPP
#define SPIRE_TIME_IN_FORCE_INPUT_WIDGET_HPP
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for selecting TimeInForce values.
  class TimeInForceInputWidget : public FilteredDropDownMenu {
    public:

      //! Constructs a TimeInForceInputWidget.
      /*!
        \param parent The parent widget.
      */
      TimeInForceInputWidget(QWidget* parent = nullptr);

      //! Returns the currently selected TimeInForce.
      Nexus::TimeInForce get_time_in_force() const;
  };
}

#endif
