#ifndef SPIRE_PERIODIC_EVALUATOR_HPP
#define SPIRE_PERIODIC_EVALUATOR_HPP
#include <utility>
#include <boost/chrono/system_clocks.hpp>
#include <QTimer>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/Evaluator.hpp"

namespace Spire::Styles {

  /**
   * Takes an Evaluator, typically the result of a <i>make_evaluator</i> call,
   * and periodically evaluates it, signalling the result of each evaluation.
   * @param <T> The type of the result of an evaluation.
   */
  template<typename T>
  class PeriodicEvaluator : private QObject {
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

      /** Constructs a PeriodicEvaluator for a specified Evaluator. */
      explicit PeriodicEvaluator(Evaluator<Type> evaluator);

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
  PeriodicEvaluator(T) -> PeriodicEvaluator<typename std::remove_reference_t<
    std::invoke_result_t<T, boost::posix_time::time_duration>>::Type>;

  template<typename T>
  PeriodicEvaluator<T>::PeriodicEvaluator(Evaluator<Type> evaluator)
    : m_evaluator(std::move(evaluator)),
      m_evaluation(m_evaluator(boost::posix_time::seconds(0))) {
    if(is_complete()) {
      return;
    }
    m_timer.setInterval(DEFAULT_EVALUATION_DURATION.total_milliseconds());
    connect(&m_timer, &QTimer::timeout, this,
      std::bind_front(&PeriodicEvaluator::on_timeout, this));
    m_start_time = boost::chrono::steady_clock::now();
    m_timer.start();
  }

  template<typename T>
  const typename PeriodicEvaluator<T>::Type&
      PeriodicEvaluator<T>::get_evaluation() const {
    return m_evaluation.m_value;
  }

  template<typename T>
  bool PeriodicEvaluator<T>::is_complete() const {
    return m_evaluation.m_next_frame == boost::posix_time::pos_infin;
  }

  template<typename T>
  boost::signals2::connection PeriodicEvaluator<T>::connect_evaluated_signal(
      const EvaluatedSignal::slot_type& slot) const {
    return m_evaluated.connect(slot);
  }

  template<typename T>
  void PeriodicEvaluator<T>::on_timeout() {
    auto duration = boost::posix_time::milliseconds(
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
