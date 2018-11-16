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

      //! Clears the items from the list and populates it with the specified
      //! items.
      /*
        \param items The items to display in the menu.
      */
      void set_items(const std::vector<QString>& items);

      //! Returns the item in the list that follows the item specified.
      /*
        \param text The specified item.
      */
      QString get_next(const QString& text);

      //! Returns the item in the list that comes before the item specified.
      /*
        \param text The specified item.
      */
      QString get_previous(const QString& text);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      std::unique_ptr<DropShadow> m_shadow;
      QScrollArea* m_scroll_area;
      QWidget* m_list_widget;
      int m_highlight_index;

      int get_index(const QString& text);
      void on_select(const QString& text);
      void focus_next();
      void focus_previous();
      void update_highlights(int old_index, int new_index);
  };
}

#endif
