#ifndef SPIRE_SIDE_INPUT_WIDGET_HPP
#define SPIRE_SIDE_INPUT_WIDGET_HPP
#include "Spire/Ui/FilteredDropDownMenu.hpp"

namespace Spire {

  //! Represents a widget for selecting Side values.
  class SideInputWidget : public FilteredDropDownMenu {
    public:

      //! Constructs a SideInputWidget.
      /*!
        \param parent The parent widget.
      */
      SideInputWidget(QWidget* parent = nullptr);

      //! Returns the currently selected Side.
      Nexus::Side get_side() const;
  };
}

#endif
