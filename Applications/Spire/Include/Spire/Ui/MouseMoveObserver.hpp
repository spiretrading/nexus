#ifndef SPIRE_MOUSE_MOVE_OBSERVER_HPP
#define SPIRE_MOUSE_MOVE_OBSERVER_HPP
#include <memory>
#include <QMouseEvent>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Monitors mouse moves on a QWidget and all of its children. */
  class MouseMoveObserver {
    public:

      /**
       * Signals a mouse move event.
       * @param target The QWidget that received the event.
       * @param event The QMouseEvent representing the event.
       */
      using MoveSignal = Signal<void (QWidget& target, QMouseEvent& event)>;

      /**
       * Constructs a MouseMoveObserver.
       * @param widget The parent widget to observe.
       */
      explicit MouseMoveObserver(QWidget& widget);

      /** Connects a slot to the move signal. */
      boost::signals2::connection connect_move_signal(
        const MoveSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      mutable MoveSignal m_move_signal;
      std::shared_ptr<EventFilter> m_filter;
      boost::signals2::scoped_connection m_connection;
  };
}

#endif
