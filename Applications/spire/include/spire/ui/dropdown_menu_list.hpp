#ifndef SPIRE_DROPDOWN_MENU_LIST_HPP
#define SPIRE_DROPDOWN_MENU_LIST_HPP
#include <QScrollArea>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace Spire {
  class DropdownMenuList : public QWidget {
    public:

      using SelectedSignal = Signal<void (const QString& text)>;

      //! Constructs a DropdownMenuList with the specified items.
      /*
        \param items The initial items in the list.
        \param parent The parent to the list.
      */
      DropdownMenuList(const std::initializer_list<QString>& items,
          QWidget* parent = nullptr);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable SelectedSignal m_selected_signal;
      std::unique_ptr<DropShadow> m_shadow;
      QScrollArea* m_scroll_area;
      QWidget* m_list_widget;

      void on_select(const QString& text);
  };
}

#endif
