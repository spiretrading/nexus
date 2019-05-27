#ifndef SPIRE_DROPDOWN_COLOR_PICKER_HPP
#define SPIRE_DROPDOWN_COLOR_PICKER_HPP
#include <QWidget>

namespace Spire {

  //! Displays a button that activates a pop-up color picker.
  class DropdownColorPicker : public QWidget {
    public:

      //! Constructs a DropdownColorPicker.
      /*
        \param parent The parent widget.
      */
      DropdownColorPicker(QWidget* parent = nullptr);
  };
}

#endif
