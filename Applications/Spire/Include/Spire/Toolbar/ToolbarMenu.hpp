#ifndef SPIRE_TOOLBAR_MENU_HPP
#define SPIRE_TOOLBAR_MENU_HPP
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Provides a drop-down menu with a title.
  class ToolbarMenu : public StaticDropDownMenu {
    public:

      //! Constructs an empty ToolbarMenu.
      /*!
        \param title The text on the top-level button.
        \param parent The parent to the ToolbarMenu.
      */
      explicit ToolbarMenu(const QString& title, QWidget* parent = nullptr);

      //! Adds an item to the menu with an icon.
      /*!
        \param text The text string for the item.
        \param icon Image for the icon.
      */
      void add(const QString& text, const QImage& icon);

      void remove_item(unsigned int index) override;
  };
}

#endif
