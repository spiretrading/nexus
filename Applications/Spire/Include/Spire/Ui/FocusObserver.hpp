#ifndef SPIRE_FOCUS_OBSERVER_HPP
#define SPIRE_FOCUS_OBSERVER_HPP
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents an observer to keep track of the focus states of a widget. */
  class FocusObserver {
    public:
      
      /**
       * Specifies the focus state.
       */
      enum class State : std::uint8_t {

        /** The state indicates a widget doesn't have focus. */
        NONE = 0,

        /** The state indicates a widget gets focus. */
        FOCUS = 0x01,

        /**
         * The state indicates a widget or one of this widget's children
         * gets focus.
         */
        FOCUS_IN = 0x02,

        /**
         * The state indicates a widget gets focus via keyboard or other
         * non-pointing device.
         */
        FOCUS_VISIBLE = 0x04
      };

      /**
       * Signals that the state has changed.
       */
      using StateSignal = Signal<void (State state)>;

      /**
       * Constructs a FocusObserver.
       * @param widget The widget tracked its focus state.
       */
      explicit FocusObserver(const QWidget& widget);

      /** Returns the current state. */
      State get_state() const;

      /** Connects a slot to the focus state signal. */
      boost::signals2::connection connect_state_signal(
        const StateSignal::slot_type& slot) const;

    private:
      struct FocusEventFilter;
      mutable StateSignal m_state_signal;
      const QWidget* m_widget;
      State m_state;
      State m_old_state;
      std::unique_ptr<FocusEventFilter> m_focus_event_filter;
  };

  /**
   * Provides a bitwise inclusive OR operator for FocusObserver::State.
   * @param left The left hand operand.
   * @param right The right hand operand.
   */
  FocusObserver::State operator |(FocusObserver::State left,
    FocusObserver::State right);

  /**
   * Provides a bitwise AND operator for FocusObserver::State.
   * @param left The left hand operand.
   * @param right The right hand operand.
   */
  FocusObserver::State operator &(FocusObserver::State left,
    FocusObserver::State right);

  /**
   * Provides a compound operator |= for FocusObserver::State.
   * @param left The left hand operand.
   * @param right The right hand operand.
   */
  FocusObserver::State& operator |=(FocusObserver::State& left,
    FocusObserver::State right);
}

#endif
