#ifndef SPIRE_DROPDOWN_MENU_ITEM_HPP
#define SPIRE_DROPDOWN_MENU_ITEM_HPP
#include <QLabel>
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Represents an item in a DropdownMenu.
  class DropdownMenuItem : public QLabel {
    public:

      using SelectedSignal = Signal<void (const QString& text)>;

      //! Constructs a DropdownMenuItem with displayed text.
      /*
        \param text The text to display.
        \param parent The parent to this widget.
      */
      DropdownMenuItem(const QString& text, QWidget* parent = nullptr);

      //! Sets the item's highlighted style.
      void set_highlight();

      //! Removes the item's highlighted style.
      void remove_highlight();

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      bool m_is_highlighted;
  };
}

#endif
