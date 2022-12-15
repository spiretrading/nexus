#ifndef SPIRE_TABLE_ITEM_HPP
#define SPIRE_TABLE_ITEM_HPP
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents an item in a table. */
  class TableItem : public QWidget {
    public:

      /** The set of styling applied to a TableItem. */
      struct Styles {

        /** The background color to use. */
        QColor m_background_color;

        /** The top border color. */
        QColor m_border_top_color;

        /** The right border color. */
        QColor m_border_right_color;

        /** The bottom border color. */
        QColor m_border_bottom_color;

        /** The left border color. */
        QColor m_border_left_color;
      };

      /** Signals that this TableItem was activated. */
      using ActiveSignal = Signal<void ()>;

      /**
       * Constructs a TableItem.
       * @param component The component to display.
       * @param parent The parent widget.
       */
      explicit TableItem(QWidget& component, QWidget* parent = nullptr);

      /** Returns the styling applied to this item. */
      const Styles& get_styles() const;

      /** Returns the body of this item. */
      const QWidget& get_body() const;

      /** Returns the body of this item. */
      QWidget& get_body();

      /** Connects a slot to the ActiveSignal. */
      boost::signals2::connection connect_active_signal(
        const ActiveSignal::slot_type& slot) const;

    private:
      mutable ActiveSignal m_active_signal;
      Button* m_button;
      Styles m_styles;
      boost::optional<FocusObserver> m_focus_observer;
      boost::signals2::scoped_connection m_style_connection;

      void on_focus(FocusObserver::State state);
      void on_style();
  };
}

#endif
