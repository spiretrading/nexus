#ifndef SPIRE_HOVER_OBSERVER_HPP
#define SPIRE_HOVER_OBSERVER_HPP
#include <QTimer>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class HoverSingleton {
    public:

      enum class State : std::uint8_t {
        NONE = 0,
        MOUSE_OVER = 0x01,
        MOUSE_IN = MOUSE_OVER | 0x02,
      };

      using StateSignal = Signal<void (State state)>;

      static HoverSingleton* instance();

      void add(const QWidget& widget);

      State get_state(const QWidget& widget) const;

      boost::signals2::connection connect_state_signal(
        const QWidget& widget,
        const std::function<void (State state)>& callback);

    private:
      struct Entry {
        State m_state;
        StateSignal m_state_signal;
      };
      HoverSingleton();
      static HoverSingleton* m_instance;
      StateSignal m_state_signal;
      std::unordered_map<const QWidget*, Entry> m_entries;
      QTimer m_poll_timer;
      QWidget* m_current;

      void on_poll_timeout();
  };

  bool is_set(HoverSingleton::State left, HoverSingleton::State right);

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
      explicit HoverObserver(const QWidget& widget);

      /** Returns the current state. */
      State get_state() const;

      /** Connects a slot to the hover state signal. */
      boost::signals2::connection connect_state_signal(
        const StateSignal::slot_type& slot) const;

    private:
      mutable StateSignal m_state_signal;
      static QTimer m_poll_timer;

      static void on_poll_timeout();
  };

  /**
   * Tests if a state is a superset of another.
   * @return <code>true</code> iff <i>left</i> is a superset of <i>right</i>.
   */
  bool is_set(HoverObserver::State left, HoverObserver::State right);
}

#endif
