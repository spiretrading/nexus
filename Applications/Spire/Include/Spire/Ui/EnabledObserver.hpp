#ifndef SPIRE_ENABLED_OBSERVER_HPP
#define SPIRE_ENABLED_OBSERVER_HPP
#include <memory>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Monitors changes to a QWidget's enabled state. */
  class EnabledObserver {
    public:

      /**
       * Signals a change to the observed QWidget's enabled state.
       * @param is_enabled <code>true</code> iff the observed QWidget is
       *        enabled.
       */
      using EnabledSignal = Signal<void (bool is_enabled)>;

      /**
       * Constructs an EnabledObserver.
       * @param widget The widget to observe.
       */
      explicit EnabledObserver(QWidget& widget);

      /** Connects a slot to the enabled signal. */
      boost::signals2::connection connect_enabled_signal(
        const EnabledSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      mutable EnabledSignal m_enabled_signal;
      std::shared_ptr<EventFilter> m_filter;
      boost::signals2::scoped_connection m_filter_connection;
  };
}

#endif
