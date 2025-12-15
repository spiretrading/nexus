#ifndef SPIRE_SUBMENU_ITEM_HPP
#define SPIRE_SUBMENU_ITEM_HPP
#include <QWidget>
#include "Spire/Ui/ContextMenu.hpp"

namespace Spire {

  /** Represents a submenu item in a ContextMenu. */
  class SubmenuItem : public QWidget {
    public:

      /**
       * Constructs a SubmenuItem.
       * @param label The label displayed on the SubmenuItem.
       * @param menu The ContextMenu component that this SubmenuItem opens.
       * @param parent The parent widget.
       */
      SubmenuItem(QString label, ContextMenu& menu, QWidget* parent = nullptr);

      /** Returns the ContextMenu component. */
      ContextMenu& get_context_menu();

    private:
      ContextMenu* m_menu;
  };
}

#endif
