#ifndef SPIRE_DROP_DOWN_MENU_2_HPP
#define SPIRE_DROP_DOWN_MENU_2_HPP
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <QVBoxLayout>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownMenuItem2.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/ScrollArea.hpp"

namespace Spire {

  //! Represents a Spire-styled drop down menu.
  class DropDownMenu2 : public QWidget {
    public:
  
      //! Signals that the current item has changed.
      /*!
        \param item The current item.
      */
      using CurrentSignal = Signal<void (const QVariant& item)>;
  
      //! Signals that an item was hovered.
      /*!
        \param item The hovered item.
      */
      using HoveredSignal = Signal<void (const QVariant& item)>;
  
      //! Signals that an item was selected.
      /*!
        \param item The selected item.
      */
      using SelectedSignal = Signal<void (const QVariant& item)>;
  
      //! Constructs a DropDownMenu.
      /*!
        \param items The items to display.
        \param parent The parent widget. Used to determine the position of the
               DropDownMenu.
      */
      explicit DropDownMenu2(std::vector<DropDownMenuItem2*> items,
        QWidget* parent = nullptr);

      //! Returns the item's value at the given index, or an invalid QVariant
      //! if the index is invalid.
      /*!
        \param index The index;
      */
      const QVariant& get_value(int index) const;

      //! Returns the current index, or none if there is no current index.
      boost::optional<int> get_current() const;
  
      //! Sets the index of the current item iff the given index is valid.
      /*!
        \param index The current index.
      */
      void set_current(int index);

      //! Returns the index of the selected item, or none if there is no
      //! selected item.
      boost::optional<int> get_selected() const;

      //! Sets the selected index iff the given index is valid.
      /*!
        \param index The selected index.
      */
      void set_selected(int index);
  
      //! Returns the number of items in the menu.
      int get_count() const;
  
      //! Connects a slot to the current changed signal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;
  
      //! Connects a slot to the hovered signal.
      boost::signals2::connection connect_hovered_signal(
        const HoveredSignal::slot_type& slot) const;

      //! Connects a slot to the selected signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    private:
      mutable CurrentSignal m_current_signal;
      mutable HoveredSignal m_hovered_signal;
      mutable SelectedSignal m_selected_signal;
      int m_max_displayed_items;
      DropShadow* m_shadow;
      QVBoxLayout* m_list_layout;
      ScrollArea* m_scroll_area;
      boost::optional<int> m_selected_index;
      boost::optional<int> m_current_index;
      Beam::SignalHandling::ConnectionGroup m_item_hovered_connections;
      Beam::SignalHandling::ConnectionGroup m_item_selected_connections;

      DropDownMenuItem2* get_item(int index) const;
      void scroll_to_current_index();
      void update_height();
      void on_item_selected(const QVariant& value, int index);
      void on_item_selected(const QVariant& value, DropDownMenuItem2* item);
  };

  //! Decrements the current index of the given DropDownMenu.
  /*!
    \param menu The DropDownMenu.
  */
  void decrement_current(DropDownMenu2& menu);

  //! Increments the current index of the given DropDownMenu.
  /*!
    \param menu The DropDownMenu.
  */
  void increment_current(DropDownMenu2& menu);

  //! Sets the current item in the given DropDownMenu as the first item in the
  //! menu that starts with the given text, iff there is a match.
  /*!
    \param menu The DropDownMenu.
    \param text The text to match against the item.
  */
  void set_current(DropDownMenu2& menu, const QString& text);
}

#endif
