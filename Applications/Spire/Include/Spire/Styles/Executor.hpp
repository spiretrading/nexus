#ifndef SPIRE_STYLES_EXECUTOR_HPP
#define SPIRE_STYLES_EXECUTOR_HPP
#include <utility>
#include <boost/chrono/system_clocks.hpp>
#include <QTimer>
#include "Spire/Spire/Spire.hpp"
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

      /** Returns the most recent evaluation. */
      const Type& get_evaluation() const;

      /** Returns <code>true</code> iff there are no further evaluations. */
      bool is_complete() const;

      /** Connects a slot to the EvaluatedSignal. */
      boost::signals2::connection connect_evaluated_signal(
        const EvaluatedSignal::slot_type& slot) const;

    private:
      mutable EvaluatedSignal m_evaluated;
      Evaluator<Type> m_evaluator;
      Evaluation<Type> m_evaluation;
      QTimer m_timer;
      boost::chrono::steady_clock::time_point m_start_time;

      void on_timeout();
  };

  template<typename T>
  Executor<T>::Executor(Evaluator<Type> evaluator)
    : m_evaluator(std::move(evaluator)),
      m_evaluation(m_evaluator(boost::posix_time::seconds(0))),
      m_frame_count(0) {
    if(is_complete()) {
      return;
    }
    m_timer.setInterval(DEFAULT_EVALUATION_DURATION.total_milliseconds());
    connect(&m_timer, &QTimer::timeout, this,
      std::bind_front(&Executor::on_timeout, this));
    m_start_time = boost::chrono::steady_clock::now();
    m_timer.start();
  }

  template<typename T>
  const typename Executor<T>::Type& Executor<T>::get_evaluation() const {
    return m_evaluation.m_value;
  }

  template<typename T>
  bool Executor<T>::is_complete() const {
    return m_evaluation.m_next_frame == boost::posix_time::pos_infin;
  }

  template<typename T>
  boost::signals2::connection Executor<T>::connect_evaluated_signal(
      const EvaluatedSignal::slot_type& slot) const {
    return m_evaluated.connect(slot);
  }

  template<typename T>
  void Executor<T>::on_timeout() {
    auto duration = boost::posix_time::millseconds(
      boost::chrono::duration_cast<boost::chrono::milliseconds>(
        boost::chrono::steady_clock::now() - m_start_time).count());
    m_evaluation.m_next_frame -= duration;
    if(m_evaluation.m_next_frame > boost::posix_time::seconds(0)) {
      return;
    }
    auto evaluation = m_evaluator(duration);
    if(is_complete()) {
      m_timer.stop();
    }
    m_evaluation = evaluation;
    m_evaluated(evaluation.m_value);
  }
}

#endif
