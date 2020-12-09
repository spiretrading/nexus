#ifndef SPIRE_DROP_DOWN_MENU_ITEM_2_HPP
#define SPIRE_DROP_DOWN_MENU_ITEM_2_HPP
#include <QVariant>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents an item in a DropDownMenu.
  class DropDownMenuItem2 : public QWidget {
    public:

      //! Signals that the item was hovered.
      using HoveredSignal = Signal<void ()>;

      //! Signals that the item was selected.
      using SelectedSignal = Signal<void ()>;

      //! Returns the item's value.
      const QVariant& get_value() const;

      //! Returns true iff the item is highlighted.
      bool is_highlighted() const;

      //! Displays the item's highlight style.
      void set_highlighted();

      //! Removes the item's highlight style.
      void reset_highlighted();
    
      //! Connects a slot to the hovered signal.
      boost::signals2::connection connect_hovered_signal(
        const HoveredSignal::slot_type& slot) const;
      
      //! Connects a slot to the selected signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      explicit DropDownMenuItem2(QVariant value, QWidget* parent = nullptr);
      void enterEvent(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      mutable HoveredSignal m_hovered_signal;
      mutable SelectedSignal m_selected_signal;
      QVariant m_value;
      bool m_is_highlighted;
  };
}

#endif
