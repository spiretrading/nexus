#ifndef SPIRE_SUB_MENU_ITEM_HPP
#define SPIRE_SUB_MENU_ITEM_HPP
#include <QWidget>
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a submenu item in a ContextMenu. */
  class SubMenuItem : public QWidget {
    public:

      /**
       * Constructs a SubMenuItem.
       * @param label The label displayed on the SubMenuItem.
       * @param menu The ContextMenu component that this SubMenuItem opens.
       * @param parent The parent widget.
       */
      SubMenuItem(QString label, ContextMenu& menu, QWidget* parent = nullptr);

      /** Returns the ContextMenu component. */
      ContextMenu& get_context_menu();

    private:
      ContextMenu* m_menu;
  };
}

#endif
