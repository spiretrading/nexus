#ifndef SPIRE_FOCUS_OBSERVER_HPP
#define SPIRE_FOCUS_OBSERVER_HPP
#include <memory>
#include <ostream>
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

        /** The state indicates a widget or any of its descendants has focus. */
        FOCUS_IN = 0x01,

        /** The state indicates a widget has focus. */
        FOCUS = FOCUS_IN | 0x02,

        /**
         * The state indicates a widget gets focus via non-pointing device or
         * through receiving programmatic focus from a component that had
         * <code>FOCUS_VISIBLE</code>.
         */
        FOCUS_VISIBLE = FOCUS | 0x04
      };

      /**
       * Signals that the state has changed.
       * @param state The changed focus state.
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
      struct ApplicationFocusFilter;
      struct FocusEventFilter;
      mutable StateSignal m_state_signal;
      std::shared_ptr<FocusEventFilter> m_filter;
      boost::signals2::scoped_connection m_filter_connection;
  };

  /** Returns the current focus state of a widget. */
  FocusObserver::State find_focus_state(const QWidget& widget);

  /**
   * Tests if a state is a superset of another.
   * @return <code>true</code> iff <i>left</i> is a superset of <i>right</i>.
   */
  bool is_set(FocusObserver::State left, FocusObserver::State right);

  std::ostream& operator <<(std::ostream& out, FocusObserver::State state);
}

#endif
