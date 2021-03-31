#ifndef SPIRE_LIST_ITEM_HPP
#define SPIRE_LIST_ITEM_HPP
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/Button.hpp"

namespace Spire {

  //! Represents an item in a list.
  class ListItem : public Styles::StyledWidget {
    public:

      //! Signals that the item received focus.
      using CurrentSignal = Signal<void ()>;
  
      //! Signals that the item was submitted.
      using SubmitSignal = Signal<void ()>;
  
      //! Constructs a ListItem.
      /*!
        \param component The component to display.
        \param parent The parent widget.
      */
      explicit ListItem(QWidget* component, QWidget* parent = nullptr);
  
      //! Connects a slot to the current signal.
      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const;
  
      //! Connects a slot to the submit signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void paintEvent(QPaintEvent* event) override {
        
        StyledWidget::paintEvent(event);
      }

    private:
      mutable SubmitSignal m_current_signal;
      Button* m_button;
  };
}

#endif
