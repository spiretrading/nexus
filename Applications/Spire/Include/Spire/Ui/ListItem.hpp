#ifndef SPIRE_LIST_ITEM_HPP
#define SPIRE_LIST_ITEM_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"

namespace Spire {
namespace Styles {

  /** Selects the selected widget. */
  using Selected = StateSelector<void, struct SelectedSelectorTag>;
}

  /** Represents an item in a list. */
  class ListItem : public QWidget {
    public:

      /** Signals that the item received focus. */
      using CurrentSignal = Signal<void ()>;
  
      /** Signals that the item was submitted. */
      using SubmitSignal = Signal<void ()>;
  
      /**
       * Constructs a ListItem.
       * @param component The component to display.
       * @param parent The parent widget.
       */
      explicit ListItem(QWidget* component, QWidget* parent = nullptr);
  
      /** Returns <code>true</code> iff this ListItem is selected. */
      bool is_selected() const;

      /**
       * Sets the selected state.
       * @param is_selected True iff the ListItem should be selected.
       */
      void set_selected(bool is_selected);

      /** Connects a slot to the current signal. */
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;
  
      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable SubmitSignal m_current_signal;
      Box* m_box;
      Button* m_button;
      bool m_is_selected;
  };
}

#endif
