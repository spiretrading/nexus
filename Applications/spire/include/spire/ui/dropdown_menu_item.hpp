#ifndef SPIRE_DROPDOWN_MENU_ITEM_HPP
#define SPIRE_DROPDOWN_MENU_ITEM_HPP
#include <QLabel>

namespace Spire {

  //! Represents an item in a DropdownMenu.
  class DropdownMenuItem : public QLabel {
    public:

      //! Constructs a DropdownMenuItem with displayed text.
      /*
        \param text The text to display.
        \param parent The parent to this widget.
      */
      DropdownMenuItem(const QString& text, QWidget* parent = nullptr);
  };
}

#endif
