#ifndef SPIRE_STYLES_CHAIN_EXPRESSION_HPP
#define SPIRE_STYLES_CHAIN_EXPRESSION_HPP
#include <memory>
#include <utility>
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Chains two Expressions together, evaluating to the first until the first
   * expression terminates, and then evaluating to the second.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class ChainExpression {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a ChainExpression.
       * @param first The first Expression to evaluate to.
       * @param second The Expression to evaluate to after <i>first</i>
       *        terminates.
       */
      ChainExpression(Expression<Type> first, Expression<Type> second);

      /** Returns the first Expression to evaluate to. */
      const Expression<Type>& get_first() const;

      /** Returns the second Expression to evaluate to. */
      const Expression<Type>& get_second() const;

      bool operator ==(const ChainExpression& expression) const;

      bool operator !=(const ChainExpression& expression) const;

    private:
      Expression<Type> m_first;
      Expression<Type> m_second;
  };

  template<typename T>
  ChainExpression(T&&, T&&) -> ChainExpression<expression_type_t<T>>;

  /**
   * Chains a series of expressions together to form a single animation.
   * @param expression The series of expressions to chain.
   * @return A ChainExpression sequencing all sub-expressions together.
   */
  template<typename T, typename... U>
  auto chain(T&& head, U&&... expression) {
    return ChainExpression(
      std::forward<T>(head), chain(std::forward<U>(expression)...));
  }

  template<typename T>
  decltype(auto) chain(T&& expression) {
    return std::forward<T>(expression);
  }

  template<typename T>
  auto make_evaluator(ChainExpression<T> expression, const Stylist& stylist) {
    using Type = T;
    struct ChainEvaluator {
      std::shared_ptr<Evaluator<Type>> m_first;
      std::shared_ptr<Evaluator<Type>> m_second;
      boost::posix_time::time_duration m_offset;
      Evaluation<Type> operator ()(boost::posix_time::time_duration frame) {
        if(m_first) {
          auto evaluation = (*m_first)(frame);
          if(evaluation.m_next_frame == boost::posix_time::pos_infin) {
            m_first = nullptr;
            m_offset = frame;
            evaluation.m_next_frame = boost::posix_time::seconds(0);
          }
          return evaluation;
        }
        auto evaluation = (*m_second)(frame - m_offset);
        if(evaluation.m_next_frame == boost::posix_time::pos_infin) {
          m_second = nullptr;
        }
        return evaluation;
      }
    };
    return ChainEvaluator{std::make_shared<Evaluator<Type>>(
      make_evaluator(expression.get_first(), stylist)),
      std::make_shared<Evaluator<Type>>(
        make_evaluator(expression.get_second(), stylist))};
  }

  template<typename T>
  ChainExpression<T>::ChainExpression(
    Expression<Type> first, Expression<Type> second)
    : m_first(std::move(first)),
      m_second(std::move(second)) {}

  template<typename T>
  const Expression<typename ChainExpression<T>::Type>&
      ChainExpression<T>::get_first() const {
    return m_first;
  }

  template<typename T>
  const Expression<typename ChainExpression<T>::Type>&
      ChainExpression<T>::get_second() const {
    return m_second;
  }

  template<typename T>
  bool ChainExpression<T>::operator ==(
      const ChainExpression& expression) const {
    return m_first == expression.m_first && m_second == expression.m_second;
  }

  template<typename T>
  bool ChainExpression<T>::operator !=(
      const ChainExpression& expression) const {
    return !(*this == expression);
  }
}

#endif
