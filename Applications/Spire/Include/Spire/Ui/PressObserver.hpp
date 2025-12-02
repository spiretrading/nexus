#ifndef SPIRE_PRESS_OBSERVER_HPP
#define SPIRE_PRESS_OBSERVER_HPP
#include <memory>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /**
   * Implements an observer signalling a press performed on a QWidget. On
   * Windows a press is performed both by a left button mouse press as well
   * as the spacebar pressed. This observer will unify both actions into a
   * single signal.
   */
  class PressObserver {
    public:

      /** Specifies the reason for the press. */
      enum class Reason {

        /** The press is due to the keyboard. */
        KEYBOARD,

        /** The press is due to the mouse. */
        MOUSE
      };

      /**
       * Signals the start of a press.
       * @param reason The reason for the press.
       */
      using PressStartSignal = Signal<void (Reason reason)>;

      /**
       * Signals the end of a press.
       * @param reason The reason for the press.
       */
      using PressEndSignal = Signal<void (Reason reason)>;

      /**
       * Constructs a PressObserver.
       * @param widget The widget to observe for presses.
       */
      explicit PressObserver(const QWidget& widget);

      ~PressObserver();

      /** Connects a slot to the press start signal. */
      boost::signals2::connection connect_press_start_signal(
        const PressStartSignal::slot_type& slot) const;

      /** Connects a slot to the press end signal. */
      boost::signals2::connection connect_press_end_signal(
        const PressEndSignal::slot_type& slot) const;

    private:
      struct PressEventFilter;
      std::unique_ptr<PressEventFilter> m_filter;
  };
}

#endif
