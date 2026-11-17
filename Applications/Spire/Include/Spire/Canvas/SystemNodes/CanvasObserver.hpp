#ifndef SPIRE_CANVAS_OBSERVER_HPP
#define SPIRE_CANVAS_OBSERVER_HPP
#include <any>
#include <memory>
#include <vector>
#include <Beam/SignalHandling/ScopedSlotAdaptor.hpp>
#include <QTimer>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Observes the value of a Canvas expression for a task. */
  class CanvasObserver {
    public:

      /**
       * Signals a change to the observed value.
       * @param value The latest value of the expression.
       */
      using UpdateSignal = Signal<void (const std::any& value)>;

      /**
       * Observes an expression in the context of a task.
       * @param task The task to observe.
       * @param observer The expression to evaluate.
       */
      CanvasObserver(std::shared_ptr<Task> task, const CanvasNode& observer);

      ~CanvasObserver();

      /** Returns the latest value, or an empty any before the first update. */
      const std::any& get_value() const;

      /**
       * Connects a slot to the UpdateSignal.
       * @param slot The slot to connect to the signal.
       * @return A connection to the signal.
       */
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const;

    private:
      std::shared_ptr<Task> m_task;
      std::unique_ptr<CanvasNode> m_observer;
      std::vector<const CanvasNode*> m_dependencies;
      std::vector<const CanvasNode*> m_remaining_dependencies;
      bool m_is_translated;
      std::any m_value;
      mutable UpdateSignal m_update_signal;
      QTimer m_translate_timer;
      Beam::ScopedSlotAdaptor m_callbacks;
      EventHandler m_event_handler;

      CanvasObserver(const CanvasObserver&) = delete;
      CanvasObserver& operator =(const CanvasObserver&) = delete;
      void translate();
      void on_reactor_update(const std::any& value);
  };
}

#endif
