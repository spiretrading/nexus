#ifndef SPIRE_HOVER_OBSERVER_HPP
#define SPIRE_HOVER_OBSERVER_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * An observer class signalling when a mouse's position intersects a QWidget.
   */
  class HoverObserver {
    public:

      /**
       * Specifies the hover state.
       */
      enum class State : std::uint8_t {

        /** The mouse is not within the widget. */
        NONE = 0,

        /** The mouse intersects the widget. */
        MOUSE_OVER = 0x01,

        /** The mouse uniquely intersects the widget (and no other). */
        MOUSE_IN = MOUSE_OVER | 0x02,
      };

      /**
       * Signals that the state has changed.
       * @param state The changed focus state.
       */
      using StateSignal = Signal<void (State state)>;

      /**
       * Constructs a HoverObserver.
       * @param widget The widget to monitor for its hover state.
       */
      explicit HoverObserver(QWidget& widget);

      /** Returns the current state. */
      State get_state() const;

      /** Connects a slot to the hover state signal. */
      boost::signals2::connection connect_state_signal(
        const StateSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      mutable StateSignal m_state_signal;
      std::shared_ptr<EventFilter> m_filter;
      boost::signals2::scoped_connection m_filter_connection;
  };

  /**
   * Tests if a state is a superset of another.
   * @return <code>true</code> iff <i>left</i> is a superset of <i>right</i>.
   */
  bool is_set(HoverObserver::State left, HoverObserver::State right);
}

#endif
