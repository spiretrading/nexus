#ifndef SPIRE_FOCUS_OBSERVER_HPP
#define SPIRE_FOCUS_OBSERVER_HPP
#include <format>
#include <memory>
#include <ostream>
#include <string_view>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

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

      ~FocusObserver();

      /** Returns the current state. */
      State get_state() const;

      /** Connects a slot to the focus state signal. */
      boost::signals2::connection connect_state_signal(
        const StateSignal::slot_type& slot) const;

    private:
      struct ApplicationFocusFilter;
      struct FocusEventFilter;
      std::shared_ptr<FocusEventFilter> m_filter;
      int m_group;

      FocusObserver(const FocusObserver&) = delete;
      FocusObserver& operator =(const FocusObserver&) = delete;
  };

  /** Returns the current focus state of a widget. */
  FocusObserver::State find_focus_state(const QWidget& widget);

  /**
   * Tests if two states have any flag in common.
   * @return <code>true</code> iff <i>left</i> and <i>right</i> share a flag.
   */
  bool is_set(FocusObserver::State left, FocusObserver::State right);

  std::ostream& operator <<(std::ostream& out, FocusObserver::State state);
}

template<>
class std::formatter<Spire::FocusObserver::State> :
    public std::formatter<std::string_view> {
  public:
    std::format_context::iterator format(
      Spire::FocusObserver::State state, std::format_context& context) const;
};

#endif
