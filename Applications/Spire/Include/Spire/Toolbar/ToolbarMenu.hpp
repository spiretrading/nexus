#ifndef SPIRE_TOOLBAR_MENU_HPP
#define SPIRE_TOOLBAR_MENU_HPP
#include <QEvent>
#include <QHash>
#include "Spire/Toolbar/Toolbar.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

namespace Spire {

  //! Provides a drop-down menu with a title.
  class ToolbarMenu : public StaticDropDownMenu {
    public:

      //! Signals that a menu item was selected.
      /*!
        \param index The index of the menu item.
      */
      using ItemSelectedSignal = Signal<void (int index)>;

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

      //! Removes an item from the menu.
      /*!
        \param index The index of the item to remove.
      */
      void remove(int index);

      //! Connects a slot to the item selected signal.
      boost::signals2::connection connect_item_selected_signal(
        const ItemSelectedSignal::slot_type& slot) const;

    private:
      mutable ItemSelectedSignal m_item_selected_signal;
      QHash<QString, int> m_item_to_index;

      void on_item_selected(const QVariant& item);
  };
}

#endif
