#ifndef SPIRE_CANVAS_OBSERVER_HPP
#define SPIRE_CANVAS_OBSERVER_HPP
#include <any>
#include <memory>
#include <vector>
#include <QTimer>
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Observes the value of a Canvas expression for a task. */
  class CanvasObserver : public ValueModel<std::any> {
    public:

      /**
       * Observes an expression in the context of a task.
       * @param task The task to observe.
       * @param observer The expression to evaluate.
       */
      CanvasObserver(std::shared_ptr<Task> task, const CanvasNode& observer);

      ~CanvasObserver() override;

      /**
       * Returns the latest observed value. Literal values are available
       * immediately. Other expressions begin empty until they yield a value.
       */
      const Type& get() const override;

      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      struct Evaluation;
      std::shared_ptr<Task> m_task;
      std::shared_ptr<Evaluation> m_evaluation;
      std::vector<const CanvasNode*> m_dependencies;
      std::vector<const CanvasNode*> m_remaining_dependencies;
      std::any m_value;
      mutable UpdateSignal m_update_signal;
      QTimer m_translate_timer;
      QtTaskQueue m_events;

      void translate();
      void on_reactor_update(const std::any& value);
  };
}

#endif
