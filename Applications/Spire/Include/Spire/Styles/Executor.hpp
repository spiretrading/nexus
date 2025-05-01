#ifndef SPIRE_STYLES_EXECUTOR_HPP
#define SPIRE_STYLES_EXECUTOR_HPP
#include <utility>
#include <QTimer>
#include "Spire/Styles/Evaluator.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Takes an Evaluator, typically the result of a <i>make_evaluator</i> call,
   * and periodically executes it, signalling the result on each evaluation.
   * @param <T> The type of the result of an evaluation.
   */
  template<typename T>
  class Executor : private QObject {
    public:

      /** The type of the result of an evaluation. */
      using Type = T;

      /**
       * Signals an evaluation.
       * @param value The result of the evaluation.
       */
      using EvaluatedSignal = Signal<void (const Type& value)>;

      /** The default time between successive evaluations. */
      static inline const auto DEFAULT_EVALUATION_DURATION =
        boost::posix_time::time_duration(boost::posix_time::seconds(1) / 60);

      /** Constructs an Executor for a specified Evaluator. */
      explicit Executor(Evaluator<Type> evaluator);

      /** Returns <code>true</code> iff there are no further evaluations. */
      bool is_complete() const;

      /** Connects a slot to the EvaluatedSignal. */
      boost::signals2::connection connect_evaluated_signal(
        const EvaluatedSignal::slot_type& slot) const;

    private:
      mutable UpdateSignal m_update;
      Evaluator<Type> m_evaluator;
      int m_frame_count;
      QTimer m_timer;

      void on_timeout();
  };

  template<typename T>
  Executor<T>::Executor(Evaluator<Type> evaluator)
    : m_evaluator(std::move(evaluator)),
      m_frame_count(0) {
    m_timer.setInterval(DEFAULT_FRAME_DURATION.total_milliseconds());
    connect(&m_timer, &QTimer::timeout, this,
      std::bind_front(&Executor::on_timeout, this));
    m_timer.start();
  }

  template<typename T>
  boost::signals2::connection Executor<T>::connect_update_signal(
      const UpdateSignal::slot_type& slot) const {
    return m_update.connect(slot);
  }

  template<typename T>
  void Executor<T>::on_timeout() {
    ++m_frame_count;
    auto evaluation = m_evaluator(DEFAULT_FRAME_DURATION * m_frame_count);
    if(evaluation.m_next_frame == pos_infin) {
      m_timer.stop();
    }
    m_update(evaluation.m_value);
  }
}

#endif
