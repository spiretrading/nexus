#ifndef SPIRE_STYLES_MINIMUM_EXPRESSION_HPP
#define SPIRE_STYLES_MINIMUM_EXPRESSION_HPP
#include "Spire/Styles/Evaluator.hpp"
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {
  template<typename T>
  using MinimumExpression =
    ExpressionDefinition<struct MinimumExpressionTag, T, T, T>;

  /**
   * Expresses the minimum of two values.
   * @param left The first value to compare.
   * @param right The second value to compare.
   * @return An Expression that evaluates to the smaller of its arguments.
   */
  template<typename T>
  auto min(Expression<T> left, Expression<T> right) {
    return MinimumExpression(std::move(left), std::move(right));
  }

  template<typename T>
  auto make_evaluator(MinimumExpression<T> expression, const Stylist& stylist) {
    using Type = T;
    struct Evaluator {
      Evaluator<Type> m_left;
      Evaluator<Type> m_right;
      Evaluation<Type> operator ()(boost::posix_time::time_duration frame) {
        auto a = m_left(frame);
        auto b = m_right(frame);
        return {std::min(a.m_value, b.m_value), std::min(a, b)};
      }
    };
    return Evaluator{
      make_evaluator(std::get<0>(expression.get_arguments()), stylist),
      make_evaluator(std::get<1>(expression.get_arguments()), stylist)};
  }
}

#endif
