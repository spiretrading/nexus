#ifndef SPIRE_STYLES_TIMEOUT_EXPRESSION_HPP
#define SPIRE_STYLES_TIMEOUT_EXPRESSION_HPP
#include <functional>
#include <memory>
#include <utility>
#include <boost/functional/hash.hpp>
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Evaluates an expression for a specified duration and then terminates.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class TimeoutExpression {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a TimeoutExpression.
       * @param expression The expression to evaluate.
       * @param duration The duration before terminating.
       */
      TimeoutExpression(
        Expression<Type> expression, boost::posix_time::time_duration duration);

      /** Returns the expression to evaluate. */
      const Expression<Type>& get_expression() const;

      /** Returns the duration of the transition. */
      boost::posix_time::time_duration get_duration() const;

      bool operator ==(const TimeoutExpression& expression) const = default;

    private:
      Expression<Type> m_expression;
      boost::posix_time::time_duration m_duration;
  };

  template<typename T>
  TimeoutExpression(T&&, boost::posix_time::time_duration) ->
    TimeoutExpression<expression_type_t<T>>;

  /**
   * Makes a TimeoutExpression.
   * @param expression The expression to evaluate.
   * @param duration The duration before terminating.
   */
  template<typename T>
  auto timeout(T&& expression, boost::posix_time::time_duration duration) {
    return TimeoutExpression(std::forward<T>(expression), duration);
  }

  template<typename T>
  auto make_evaluator(TimeoutExpression<T> expression, const Stylist& stylist) {
    using Type = T;
    struct TimeoutEvaluator {
      Evaluator<Type> m_evaluator;
      boost::posix_time::time_duration m_duration;

      Evaluation<Type> operator ()(boost::posix_time::time_duration frame) {
        auto evaluation = m_evaluator(std::min(frame, m_duration));
        if(frame >= m_duration) {
          evaluation.m_next_frame = boost::posix_time::pos_infin;
        } else if(evaluation.m_next_frame + frame >= m_duration) {
          evaluation.m_next_frame = m_duration - frame;
        }
        return evaluation;
      }
    };
    return TimeoutEvaluator{
      make_evaluator(expression.get_expression(), stylist),
      expression.get_duration()};
  }

  template<typename T>
  TimeoutExpression<T>::TimeoutExpression(Expression<Type> expression,
    boost::posix_time::time_duration duration)
    : m_expression(std::move(expression)),
      m_duration(duration) {}

  template<typename T>
  const Expression<typename TimeoutExpression<T>::Type>&
      TimeoutExpression<T>::get_expression() const {
    return m_expression;
  }

  template<typename T>
  boost::posix_time::time_duration TimeoutExpression<T>::get_duration() const {
    return m_duration;
  }
}

namespace std {
  template<typename T>
  struct hash<Spire::Styles::TimeoutExpression<T>> {
    std::size_t operator ()(
        const Spire::Styles::TimeoutExpression<T>& expression) const {
      auto seed = std::size_t(0);
      boost::hash_combine(seed,
        std::hash<Spire::Styles::Expression<T>>()(expression.get_expression()));
      boost::hash_combine(seed, std::hash<boost::posix_time::time_duration>()(
        expression.get_duration()));
      return seed;
    }
  };
}

#endif
