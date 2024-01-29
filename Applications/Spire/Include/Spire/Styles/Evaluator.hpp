#ifndef SPIRE_STYLES_EVALUATOR_HPP
#define SPIRE_STYLES_EVALUATOR_HPP
#include <functional>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Stores the evaluation of an Expression.
   * @param <T> The type that was evaluated.
   */
  template<typename T>
  struct Evaluation {

    /** The type that was evaluated. */
    using Type = T;

    /** The value the Expression evaluated to. */
    Type m_value;

    /**
     * Specifies a lower bound on the next frame in the evaluation.
     * Set to boost::posix_time::seconds(0) for a continuous animation or
     * boost::posix_time::pos_infin if the evaluation is complete or to the
     * time delay between two successive evaluations.
     */
    boost::posix_time::time_duration m_next_frame;

    /**
     * Constructs an Evaluation that has no further frame.
     * @param value The value to evaluate to.
     */
    explicit Evaluation(Type value);

    /**
     * Constructs an Evaluation that has no further frame.
     * @param value The value to evaluate to.
     * @param next_frame The lower bound on the next frame in the evaluation.
     */
    Evaluation(Type value, boost::posix_time::time_duration next_frame);
  };

  template<typename T>
  using Evaluator =
    std::function<Evaluation<T> (boost::posix_time::time_duration)>;

  template<typename T>
  Evaluation<T>::Evaluation(Type value)
    : Evaluation(std::move(value), boost::posix_time::pos_infin) {}

  template<typename T>
  Evaluation<T>::Evaluation(Type value,
      boost::posix_time::time_duration next_frame)
    : m_value(std::move(value)),
      m_next_frame(next_frame) {}
}

#endif
