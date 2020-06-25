#ifndef SPIRE_DROP_DOWN_MENU_ITEM_HPP
#define SPIRE_DROP_DOWN_MENU_ITEM_HPP
#include <QLabel>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents an item in a DropDownMenu.
  class DropDownMenuItem : public QLabel {
    public:

      //! Signals that an user selected an item.
      /*!
        \param text The item selected.
      */
      using SelectedSignal = Signal<void (const QString& text)>;

      //! Constructs a DropDownMenuItem with displayed text.
      /*!
        \param text The text to display.
        \param parent The parent to this widget.
      */
      explicit DropDownMenuItem(const QString& text, QWidget* parent = nullptr);

      //! Sets the item's highlighted style.
      void set_highlight();

      //! Removes the item's highlighted style.
      void remove_highlight();

      //! Calls the provided slot when the selected signal is triggered.
      /*!
        \param slot The slot to call.
      */
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
