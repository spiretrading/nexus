#ifndef SPIRE_DROP_DOWN_LIST_HPP
#define SPIRE_DROP_DOWN_LIST_HPP
#include <QVBoxLayout>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownItem.hpp"
#include "Spire/Ui/DropDownWindow.hpp"

namespace Spire {

  //! Represents a pop-up drop down list.
  class DropDownList : public DropDownWindow {
    public:

      //! Signals that an item was highlighted using the keyboard.
      /*!
        \param value The item's value.
      */
      using ActivatedSignal = Signal<void (const QVariant& value)>;

      //! Signals that an item was highlighted using the mouse.
      /*!
        \param value The item's value.
      */
      using HighlightedSignal = Signal<void (const QVariant& value)>;

      //! Signals that an item was selected.
      /*!
        \param index The item's list index.
      */
      using IndexSelectedSignal = Signal<void (int index)>;

      //! Signals that an item was selected.
      /*!
        \param value The item's value.
      */
      using ValueSelectedSignal = Signal<void (const QVariant& value)>;

      //! Constructs a DropDownList.
      /*!
        \param items The items to display.
        \param is_click_activated True if the DropDownList should be shown
                                  or hidden in response to parent widget
                                  mouse/keyboard activation events, false
                                  otherwise.
        \param parent The parent widget. Used to determine the position of the
                      DropDownList.
      */
      DropDownList(std::vector<DropDownItem*> items,
        bool is_click_activated, QWidget* parent = nullptr);

      //! Returns the value of the item at the given index.
      /*!
        \param index The index of the item.
      */
      QVariant get_value(int index);

      //! Appends an item to the list.
      /*!
        \param item The item to append to the list.
      */
      void insert_item(DropDownItem* item);

      //! Returns the number of items in the list.
      int item_count() const;

      //! Removes and deletes an item.
      /*!
        \param index The index of the item to remove.
      */
      void remove_item(int index);

      //! Highlights the next item in the list that starts with the given text.
      /*!
        \param text The text to match against the next item.
      */
      void set_highlight(const QString& text);

      //! Sets the items to display, overwriting any existing items.
      /*!
        \param items The new items to display.
      */
      void set_items(std::vector<DropDownItem*> items);

      //! Connects a slot to the activated signal.
      boost::signals2::connection connect_activated_signal(
        const ActivatedSignal::slot_type& slot) const;
      
      //! Connects a slot to the highlighted signal.
      boost::signals2::connection connect_highlighted_signal(
        const HighlightedSignal::slot_type& slot) const;
      
      //! Connects a slot to the index selected signal.
      boost::signals2::connection connect_index_selected_signal(
        const IndexSelectedSignal::slot_type& slot) const;
      
      //! Connects a slot to the value selected signal.
      boost::signals2::connection connect_value_selected_signal(
        const ValueSelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable ActivatedSignal m_activated_signal;
      mutable HighlightedSignal m_highlighted_signal;
      mutable IndexSelectedSignal m_index_selected_signal;
      mutable ValueSelectedSignal m_value_selected_signal;
      int m_max_displayed_items;
      QVBoxLayout* m_layout;
      ScrollArea* m_scroll_area;
      boost::optional<int> m_highlight_index;
      CustomVariantItemDelegate m_item_delegate;
      std::vector<boost::signals2::scoped_connection>
        m_item_selected_connections;

      DropDownItem* get_widget(int index);
      void focus_next();
      void focus_previous();
      void set_highlight(int index);
      void scroll_to_highlight();
      void update_height();
      void on_item_selected(QVariant value, int index);
  };
}

#endif
