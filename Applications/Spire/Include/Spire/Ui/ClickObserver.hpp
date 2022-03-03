#ifndef SPIRE_CLICK_OBSERVER_HPP
#define SPIRE_CLICK_OBSERVER_HPP
#include <memory>
#include "Spire/Ui/PressObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements an observer signalling a click performed on a QWidget. On
   * Windows a click is performed both by a left button mouse click as well
   * as the spacebar press and release. This observer will unify both actions
   * into a single signal.
   */
  class ClickObserver {
    public:

      /** Signals a click. */
      using ClickSignal = Signal<void ()>;

      /**
       * Constructs a ClickObserver.
       * @param widget The widget to observe for clicks.
       */
      explicit ClickObserver(const QWidget& widget);

      /** Connects a slot to the click signal. */
      boost::signals2::connection connect_click_signal(
        const ClickSignal::slot_type& slot) const;

    private:
      mutable ClickSignal m_click_signal;
      const QWidget* m_widget;
      PressObserver m_press_observer;

      void on_press_end(PressObserver::Reason reason);
  };
}

#endif
