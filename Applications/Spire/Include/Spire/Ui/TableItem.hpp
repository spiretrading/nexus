#ifndef SPIRE_TABLE_ITEM_HPP
#define SPIRE_TABLE_ITEM_HPP
#include <QWidget>
#include "Spire/Ui/ClickObserver.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/MouseObserver.hpp"

class QSpacerItem;

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
       * Constructs a mounted TableItem.
       * @param body The body to mount.
       * @param parent The parent widget.
       */
      explicit TableItem(QWidget& body, QWidget* parent = nullptr);

      /**
       * Constructs an unmounted TableItem.
       * @param parent The parent widget.
       */
      explicit TableItem(QWidget* parent = nullptr);

      ~TableItem() override;

      /** Returns the styling applied to this item. */
      const Styles& get_styles() const;

      /** Returns the body of this item. */
      const QWidget& get_body() const;

      /** Returns the body of this item. */
      QWidget& get_body();

      /** Connects a slot to the ActiveSignal. */
      boost::signals2::connection connect_active_signal(
        const ActiveSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    private:
      friend class TableBody;
      mutable ActiveSignal m_active_signal;
      Styles m_styles;
      ClickObserver m_click_observer;
      FocusObserver m_focus_observer;
      MouseObserver m_mouse_observer;
      boost::signals2::scoped_connection m_focus_connection;
      boost::signals2::scoped_connection m_style_connection;

      void mount(QWidget& body);
      QWidget* unmount();
      void on_focus(FocusObserver::State state);
      void on_mouse(QWidget& target, const QMouseEvent& event);
      void on_style();
  };
}

#endif
