#ifndef SPIRE_DROPDOWN_MENU_LIST_HPP
#define SPIRE_DROPDOWN_MENU_LIST_HPP
#include <QScrollArea>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace Spire {
  class DropdownMenuList : public QWidget {
    public:

      //! Constructs a DropdownMenuList with the specified items.
      /*
        \param items The initial items in the list.
        \param parent The parent to the list.
      */
      DropdownMenuList(const std::initializer_list<QString>& items,
          QWidget* parent = nullptr);

    private:
      std::unique_ptr<DropShadow> m_shadow;
      QScrollArea* m_scroll_area;
      QWidget* m_list_widget;
  };
}

#endif
