#ifndef SPIRE_DROP_DOWN_MENU_2_HPP
#define SPIRE_DROP_DOWN_MENU_2_HPP
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <QVBoxLayout>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownItem.hpp"
#include "Spire/Ui/DropDownWindow.hpp"
#include "Spire/Ui/ScrollArea.hpp"

namespace Spire {

  class DropDownMenu2 : public DropDownWindow {
    public:

      //! Signals that the menu was closed.
      using ClosedSignal = Signal<void ()>;
  
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
      explicit DropDownMenu2(std::vector<DropDownItem*> items,
        QWidget* parent = nullptr);
  
      //! Increments the index of the current item iff the menu is not empty.
      void increment_current();

      //! Returns the current item.
      const QVariant& get_current_item() const;

      //! Returns the index of the currently selected item iff the menu is not
      //! empty.
      boost::optional<int> get_selected_index() const;
  
      //! Returns the number of items in the menu.
      int item_count() const;

      //! Returns the value of the item at the given index.
      /*!
        \param index The index of the item.
      */
      const QVariant& get_value(int index) const;

      //! Appends an item to the menu.
      /*!
        \param item The item to append to the menu.
      */
      void insert_item(DropDownItem* item);

      //! Removes and deletes an item.
      /*!
        \param index The index of the item to remove.
      */
      void remove_item(int index);

      //! Sets the current item as the first item in the menu that starts with
      //! the given text.
      /*!
        \param text The text to match against the item.
        \return True iff a match was found.
      */
      bool set_current_item(const QString& text);

      //! Sets the items to display, overwriting any existing items.
      /*!
        \param items The new items to display.
      */
      void set_items(const std::vector<DropDownItem*>& items);

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;
  
      //! Connects a slot to the current changed signal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;
  
      //! Connects a slot to the hovered signal.
      boost::signals2::connection connect_hovered_signal(
        const HoveredSignal::slot_type& slot) const;

      //! Connects a slot to the selected signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable ClosedSignal m_closed_signal;
      mutable CurrentSignal m_current_signal;
      mutable HoveredSignal m_hovered_signal;
      mutable SelectedSignal m_selected_signal;
      int m_max_displayed_items;
      QVBoxLayout* m_layout;
      ScrollArea* m_scroll_area;
      boost::optional<int> m_selected_index;
      boost::optional<int> m_current_index;
      CustomVariantItemDelegate m_item_delegate;
      Beam::SignalHandling::ConnectionGroup m_item_hovered_connections;
      Beam::SignalHandling::ConnectionGroup m_item_selected_connections;

      void decrement_current();
      DropDownItem* get_item(int index) const;
      void set_current_index(int index);
      void scroll_to_current_index();
      void update_height();
      void on_item_selected(const QVariant& value, int index);
      void on_item_selected(const QVariant& value, DropDownItem* item);
  };
}

#endif
