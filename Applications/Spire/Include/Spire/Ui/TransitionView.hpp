#ifndef SPIRE_TRANSITION_VIEW_HPP
#define SPIRE_TRANSITION_VIEW_HPP
#include <QWidget>

namespace Spire {

  /** Displays a transition animation on top of another widget. */
  class TransitionView : public QWidget {
    public:

      /** Specifies the loading status of data within body. */
      enum class Status {

        /** No data is available. */
        NONE,

        /** Data is being loaded. */
        LOADING,

        /** Data has loaded and is ready to display. */
        READY
      };

      /**
       * Constructs a TransitionView.
       * @param body The component to display when status is READY.
       * @param parent The parent widget.
       */
      explicit TransitionView(QWidget* body, QWidget* parent = nullptr);

      /** Returns the loading status of data. */
      Status get_status() const;

      /** Sets the loading status of the component data.
       *  @param status The status of data.
       */
      void set_status(Status status);

      /** Sets the body widget.
       *  @param body The new body widget.
       */
      void set_body(QWidget& body);

    private:
      QWidget* m_body;
      QWidget* m_pending_widget;
      QMovie* m_spinner;
      QTimer* m_timer;
      Status m_status;

      void add_widget_to_layout(QWidget& widget);
      void clear_layout();
      void make_pending_widget();
      void on_timer_expired();
  };
}

#endif
