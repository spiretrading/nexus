#ifndef SPIRE_GLOBAL_POSITION_OBSERVER_HPP
#define SPIRE_GLOBAL_POSITION_OBSERVER_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** An observer class signalling the global position of a QWidget. */
  class GlobalPositionObserver {
    public:

      /**
       * Signals a change in a QWidget's global position.
       * @param position The global position of the observed QWidget.
       */
      using PositionSignal = Signal<void (QPoint position)>;

      /**
       * Constructs a GlobalPositionObserver.
       * @param widget The widget whose position is to be monitored.
       */
      explicit GlobalPositionObserver(QWidget& widget);

      /** Returns the current global position. */
      QPoint get_position() const;

      /** Connects a slot to the position signal. */
      boost::signals2::connection connect_position_signal(
        const PositionSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      mutable PositionSignal m_position_signal;
      std::shared_ptr<EventFilter> m_filter;
      boost::signals2::scoped_connection m_position_connection;
  };
}

#endif
