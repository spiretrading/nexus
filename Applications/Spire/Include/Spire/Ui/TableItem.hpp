#ifndef SPIRE_TABLE_ITEM_HPP
#define SPIRE_TABLE_ITEM_HPP
#include <QWidget>
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents an item in a table. */
  class TableItem : public QWidget {
    public:

      /** The set of styling applied to a TableItem. */
      struct Styles {

        /** The background color to use. */
        QColor m_background_color;
      };

      /** Signals that this TableItem was clicked on. */
      using ClickedSignal = Button::ClickedSignal;

      /**
       * Constructs a TableItem.
       * @param component The component to display.
       * @param parent The parent widget.
       */
      explicit TableItem(QWidget& component, QWidget* parent = nullptr);

      /** Returns the styling applied to this item. */
      const Styles& get_styles() const;

      /** Connects a slot to the ClickedSignal. */
      boost::signals2::connection connect_clicked_signal(
        const ClickedSignal::slot_type& slot) const;

    private:
      Button* m_button;
      Styles m_styles;
      boost::signals2::scoped_connection m_style_connection;

      void on_style();
  };
}

#endif