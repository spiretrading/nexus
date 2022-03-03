#ifndef SPIRE_PRESS_OBSERVER_HPP
#define SPIRE_PRESS_OBSERVER_HPP
#include <memory>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements an observer signalling a press performed on a QWidget. On
   * Windows a press is performed both by a left button mouse press as well
   * as the spacebar pressed. This observer will unify both actions into a
   * single signal.
   */
  class PressObserver {
    public:

      /** Signals a press. */
      using PressSignal = Signal<void ()>;

      /**
       * Constructs a PressObserver.
       * @param widget The widget to observe for presses.
       */
      explicit PressObserver(const QWidget& widget);

      ~PressObserver();

      /** Connects a slot to the press signal. */
      boost::signals2::connection connect_press_signal(
        const PressSignal::slot_type& slot) const;

    private:
      struct PressEventFilter;
      std::unique_ptr<PressEventFilter> m_filter;
  };
}

#endif
